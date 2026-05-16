#include "Boss.hpp"
#include "Player.hpp"
#include "Constants.h"
#include "ImageManager.hpp"
#include "BossPattern.hpp"
#include <SDL.h>
#include <math.h>

//보스 초기화
void InitBoss(BossData* b) {
    b->x = (float)(SCREEN_WIDTH / 2 - BOSS_SIZE / 2);
    b->startY = -BOSS_SIZE;
    b->y = b->startY;
    b->targetY = 100.0f;
    b->rotation = 0.0f;
    b->hp = BOSS_MAX_HP;
    b->visualHp = 0.0f;
    b->sizeScale = 1.0f;
    b->state = B_PHASE1_INTRO;
    b->phase = 1;
    b->isInvincible = true;
    b->lastFireTick = SDL_GetTicks();
    b->moveTimer = BOSS_MOVE_INTERVAL;
    b->drawRect.w = BOSS_SIZE;
    b->drawRect.h = BOSS_SIZE;
    b->drawRect.x = (int)b->x;
    b->drawRect.y = (int)b->y;
}

//보스 업데이트(상태 변환, 페이즈 변경 모두 담당)
void UpdateBoss(BossData* b, PlayerData* player, float deltaTime) {
    if (b->state == B_DEAD) return;
    Uint32 currentTime = SDL_GetTicks();

    //페이즈 2 전환 조건 체크
    if (b->phase == 1 && b->hp <= (BOSS_MAX_HP * 0.01f)) {
        if (b->state != B_PHASE1to2) {
            b->state = B_PHASE1to2;
            b->stateTimer = 1.5f;
            b->isInvincible = true;
        }
    }

    //각 상태 호출
    switch (b->state) {
    case B_PHASE1_INTRO:
        Pattern_Intro(b, deltaTime);
        break;

    case B_IDLE:
        Pattern_Idle(b, player, currentTime, deltaTime);
        break;

    case B_TELEPORT_OUT:
        Pattern_TeleportOut(b, deltaTime);
        break;

    case B_TELEPORT_IN:
        Pattern_TeleportIn(b, currentTime, deltaTime);
        break;

    case B_MELEE_ATTACK:
        Pattern_MeleeAttack(b, player, deltaTime);
        break;

    case B_PHASE1to2:
        Pattern_Phase1to2(b, deltaTime);
        break;

    //레이저 딜레이 이후 바로 레이저 패턴으로 이동
    case B_LASER_DELAY:
    case B_LASER_PATTERN:
        Pattern_LaserAndDelay(b, player, currentTime, deltaTime);
        break;

    case B_PHASE2_INTRO:
        Pattern_Phase2Intro(b, deltaTime);
        break;

    case B_PHASE2_MAIN:
        b->isWarning = false;
        Pattern_SwayRoad(b, deltaTime);
        //2페이즈 종료 후 여러 값들 초기화
        if (b->hp <= 0) {
            b->phase = 3;
            b->hp = 1;
            b->visualHp = 1.0f;
            b->state = B_PHASE3_INTRO;
            b->greenBulletCount = -1;   //3페이즈 인트로용 플래그 활성화
        }
        break;

    case B_PHASE3_INTRO:
        Pattern_Phase3Intro(b, player, deltaTime);
        break;

    case B_PHASE3_MAIN:
        Pattern_Phase3Main(b, player, deltaTime);
        break;

    case B_PHASE3_OUTRO:
        Pattern_Phase3Outro(b, player, deltaTime);
        break;
    }

    //보스 크기 및 drawRect 공통 연산
    int currentSize = (int)(BOSS_SIZE * b->sizeScale);
    if (currentSize <= 0 && b->sizeScale > 0.1f) currentSize = BOSS_SIZE;
    b->drawRect.w = currentSize;
    b->drawRect.h = currentSize;
    b->drawRect.x = (int)(b->x + (BOSS_SIZE - currentSize) / 2);
    b->drawRect.y = (int)(b->y + (BOSS_SIZE - currentSize) / 2);
}

void DrawBoss(SDL_Renderer* renderer, BossData* b) {
    if (b->state == B_DEAD) return;

    if (b->state == B_PHASE3_MAIN && b->isWarning) {
        float centerX = b->x + BOSS_SIZE / 2.0f;
        float centerY = b->y + BOSS_SIZE / 2.0f;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150); //돌진 경고용 반투명 빨간 선
        SDL_RenderDrawLine(renderer, (int)centerX, (int)centerY,
            (int)(centerX + cosf(b->rushAngle) * 1200),
            (int)(centerY + sinf(b->rushAngle) * 1200));
    }

    //레이저 출력 부분
    if (b->state == B_LASER_PATTERN) {
        for (int i = -BOSS_LASER_STEP; i < SCREEN_WIDTH + BOSS_LASER_STEP; i += BOSS_LASER_STEP) {
            SDL_Rect lRect;
            float pos = i + b->laserOffset;
            if (b->isVertical) lRect = { (int)pos, 0, BOSS_LASER_THICKNESS, SCREEN_HEIGHT };
            else lRect = { 0, (int)pos, SCREEN_WIDTH, BOSS_LASER_THICKNESS };
            if (b->laserCycle < BOSS_LASER_WARNING_TIME) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 70);
                SDL_RenderFillRect(renderer, &lRect);
            }
            else if (b->laserCycle < BOSS_LASER_ACTIVE_TIME) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &lRect);
            }
        }
    }

    //본체 출력 부분
    if (b->sizeScale > 0.001f && gBossTexture != nullptr) {
        Uint8 r = 255, g = 255, bl = 255;
        if (b->hitTimer > 0) { r = 255; g = 100; bl = 100; }
        else if (b->state == B_MELEE_ATTACK) { r = 255; g = 50; bl = 50; }
        else if (b->isWarning) {
            float flash = (sinf(SDL_GetTicks() * 0.015f) + 1.0f) / 2.0f;
            r = 255; g = 200 + (Uint8)(flash * 55); bl = 0;
        }
        else if (b->state == B_PHASE1to2) { r = 255; g = 255; bl = 0; }

        SDL_SetTextureColorMod(gBossTexture, r, g, bl);
        SDL_RenderCopyEx(renderer, gBossTexture, NULL, &b->drawRect, (double)b->rotation, NULL, SDL_FLIP_NONE);
        SDL_SetTextureColorMod(gBossTexture, 255, 255, 255);

        if (b->state == B_PHASE3_OUTRO) {
            SDL_Rect rockRect = { (int)b->rockX, (int)b->rockY, 50, 50 };
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); //회색 바위
            SDL_RenderFillRect(renderer, &rockRect);
        }

        //즉사기 대폭발
        if (b->state == B_PHASE3_OUTRO && b->stateTimer <= 0.1f) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //흰색
            SDL_Rect fullScreen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
            SDL_RenderFillRect(renderer, &fullScreen);
        }
    }

    //보스 체력 바 출력 부분
    float displayHp = (b->visualHp < 0) ? 0 : b->visualHp;
    float hpRatio = displayHp / (float)BOSS_MAX_HP;
    SDL_Rect hpBg = { 100, 30, SCREEN_WIDTH - 200, 20 };
    SDL_Rect hpFill = { 100, 30, (int)((SCREEN_WIDTH - 200) * hpRatio), 20 };
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &hpBg);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &hpFill);
}
