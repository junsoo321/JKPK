#include "Boss.hpp"
#include "Projectile.hpp"
#include "Constants.h"
#include "Player.hpp"
#include "ImageManager.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern float gShakeAmount;

//각도를 주면 해당 방향으로 정확히 날아가게 하는 함수
void FireBossAngle(float startX, float startY, float angleDegree) {
    float rad = angleDegree * (M_PI / 180.0f);
    //방향 벡터 계산
    float tX = startX + cosf(rad) * 100.0f;
    float tY = startY + sinf(rad) * 100.0f;
    FireEnemyProjectile(startX, startY, tX, tY);
}

//2페이즈 탄막 패턴 함수
void Pattern_SwayRoad(BossData* b, float deltaTime) {
    static float baseAngle = 90.0f;
    static float fireTimer = 0;
    static float swayTime = 0;
    fireTimer += deltaTime;
    swayTime += deltaTime;
    if (fireTimer >= SWAY_ROAD_FIRE_INTERVAL) {
        float centerX = b->x + (BOSS_SIZE / 2.0f);
        float centerY = b->y + (BOSS_SIZE / 2.0f);
        float swayOffset = sinf(swayTime * SWAY_ROAD_SWAY_SPEED) * SWAY_ROAD_SWAY_RANGE;
        for (int i = 0; i < SWAY_ROAD_BULLET_COUNT; i++) {
            float finalAngle = baseAngle + (i * (360.0f / SWAY_ROAD_BULLET_COUNT)) + swayOffset;
            FireBossAngle(centerX, centerY, finalAngle);
        }
        fireTimer = 0;
    }
}

//보스 초기값 설정 함수
void InitBoss(BossData* b) {
    b->x = (float)(SCREEN_WIDTH / 2 - BOSS_SIZE / 2);
    b->startY = -BOSS_SIZE;
    b->y = b->startY;
    b->targetY = 100.0f;
    b->rotation = 0.0f;
    b->hp = BOSS_MAX_HP;
    b->visualHp = 0.0f;
    b->sizeScale = 1.0f;
    b->state = B_INTRO;
    b->phase = 1;
    b->isInvincible = true;
    b->lastFireTick = SDL_GetTicks();
    b->moveTimer = BOSS_MOVE_INTERVAL;
    b->drawRect.w = BOSS_SIZE;
    b->drawRect.h = BOSS_SIZE;
    b->drawRect.x = (int)b->x;
    b->drawRect.y = (int)b->y;
}

//보스와 관련된 모든 동작 처리 함수
void UpdateBoss(BossData* b, PlayerData* player, float deltaTime) {
    if (b->state == B_DEAD) return;
    Uint32 currentTime = SDL_GetTicks();

    if (b->phase == 1 && b->hp <= (BOSS_MAX_HP * 0.01f)) {
        if (b->state != B_PHASE_TRANSITION) {
            b->state = B_PHASE_TRANSITION;
            b->stateTimer = 1.5f;
            b->isInvincible = true;
        }
    }

    switch (b->state) {
    case B_INTRO:
        b->visualHp += (BOSS_MAX_HP / BOSS_INTRO_TIME) * deltaTime;
        b->isWarning = false;
        if (b->y < b->targetY) {
            b->y += (b->targetY - b->startY) / BOSS_INTRO_TIME * deltaTime;
        }
        if (b->visualHp >= BOSS_MAX_HP) {
            b->visualHp = (float)BOSS_MAX_HP;
            b->y = b->targetY;
            b->isInvincible = false;
            b->state = B_IDLE;
            b->lastFireTick = SDL_GetTicks();
        }
        break;

    case B_IDLE: {
        float dx = (player->x + PLAYER_SIZE / 2) - (b->x + BOSS_SIZE / 2);
        float dy = (player->y + PLAYER_SIZE / 2) - (b->y + BOSS_SIZE / 2);
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance <= BOSS_MELEE_RANGE) {
            if (b->meleeCooldown <= 0) {
                b->state = B_MELEE_ATTACK;
                b->stateTimer = BOSS_MELEE_PREP_TIME;
                b->isWarning = false;
                break;
            }
        }
        b->isWarning = (distance <= BOSS_MELEE_RANGE * 1.3f && b->meleeCooldown <= 0);

        if (currentTime - b->lastFireTick > (Uint32)(BOSS_ATTACK_SPEED * 1000.0f)) {
            FireEnemyProjectile(b->x + BOSS_SIZE / 2, b->y + BOSS_SIZE / 2, player->x, player->y);
            b->lastFireTick = currentTime;
        }
        b->moveTimer -= deltaTime;
        if (b->moveTimer <= 0) {
            b->state = B_TELEPORT_OUT;
            b->stateTimer = BOSS_MOVE_TP_TIME;
            b->targetX = (float)(100 + rand() % (SCREEN_WIDTH - 200));
            b->targetY = (float)(100 + rand() % (SCREEN_HEIGHT / 2));
        }
        break;
    }

    case B_TELEPORT_OUT:
        b->stateTimer -= deltaTime;
        b->sizeScale = b->stateTimer / BOSS_MOVE_TP_TIME;
        if (b->stateTimer <= 0) {
            b->sizeScale = 0.0f;
            b->x = b->targetX; b->y = b->targetY;
            b->state = B_TELEPORT_IN;
            b->stateTimer = 0.0f;
        }
        break;

    case B_TELEPORT_IN:
        b->stateTimer += deltaTime;
        b->sizeScale = b->stateTimer / BOSS_MOVE_TP_TIME;
        if (b->sizeScale >= 1.0f) {
            b->sizeScale = 1.0f;
            b->state = B_IDLE;
            b->moveTimer = BOSS_MOVE_INTERVAL;
            b->lastFireTick = currentTime;
        }
        break;

    case B_MELEE_ATTACK:
        b->stateTimer -= deltaTime;
        b->rotation += (500.0f) * deltaTime;
        if (b->stateTimer <= 0) {
            if (!player->isInvincible) {
                player->hp -= BOSS_MELEE_DAMAGE;
                player->isInvincible = true;
                player->invincibleEndTime = SDL_GetTicks() + 1000;
            }
            b->state = B_IDLE;
            b->rotation = 0;
            b->moveTimer = 1.0f;
            b->meleeCooldown = 2.0f;
            b->lastFireTick = SDL_GetTicks();
        }
        break;

    case B_PHASE_TRANSITION: {
        b->stateTimer -= deltaTime;
        float progress = 1.0f - (b->stateTimer / 1.5f);
        b->rotation += (BOSS_TRANSITION_ROTATION_BASE + progress * BOSS_TRANSITION_ROTATION_ACCEL) * deltaTime;
        b->sizeScale = powf(b->stateTimer / 1.5f, 2.0f);
        if (b->stateTimer <= 0) {
            b->state = B_LASER_DELAY;
            b->stateTimer = 1.0f;
            b->sizeScale = 0.0f;
            b->rotation = 0.0f;
            b->laserCycle = 0.0f;
            b->phase = 2;
        }
        break;
    }

    case B_LASER_DELAY:
        b->stateTimer -= deltaTime;
        b->sizeScale = 0.0f;
        if (b->stateTimer <= 0) {
            b->state = B_LASER_PATTERN;
            b->stateTimer = 13.0f;
            b->laserCycle = 0.0f;
            gShakeAmount = 0;
        }
        break;

    case B_LASER_PATTERN:
        b->stateTimer -= deltaTime;
        b->laserCycle += deltaTime;
        b->sizeScale = 0.0f;
        if (b->laserCycle >= BOSS_LASER_WARNING_TIME && b->laserCycle < BOSS_LASER_WARNING_TIME + (deltaTime * 2.0f)) {
            if (b->stateTimer < 10.0f && gShakeAmount <= 0) {
                gShakeAmount = SHAKE_INTENSITY;
            }
        }
        if (b->laserCycle >= BOSS_LASER_CYCLE_TIME) {
            b->laserCycle = 0.0f;
            b->isVertical = rand() % 2;
            b->laserOffset = (float)(rand() % 80);
        }
        if (b->laserCycle >= BOSS_LASER_WARNING_TIME && b->laserCycle <= BOSS_LASER_ACTIVE_TIME && !player->isInvincible) {
            for (int i = -BOSS_LASER_STEP; i < SCREEN_WIDTH + BOSS_LASER_STEP; i += BOSS_LASER_STEP) {
                float pos = i + b->laserOffset;
                SDL_Rect lRect;
                if (b->isVertical) lRect = { (int)pos, 0, BOSS_LASER_THICKNESS, SCREEN_HEIGHT };
                else lRect = { 0, (int)pos, SCREEN_WIDTH, BOSS_LASER_THICKNESS };
                SDL_Rect pRect = { (int)player->x, (int)player->y, PLAYER_SIZE, PLAYER_SIZE };
                if (SDL_HasIntersection(&pRect, &lRect)) {
                    player->hp -= 10;
                    player->isInvincible = true;
                    player->invincibleEndTime = currentTime + 2000;
                    break;
                }
            }
        }
        if (b->stateTimer <= 0) {
            b->state = B_PHASE2_INTRO;
            b->visualHp = 0;
            b->sizeScale = 1.0f;
            b->x = (SCREEN_WIDTH / 2.0f) - (BOSS_SIZE / 2.0f);
            b->y = 100.0f;
        }
        break;

    case B_PHASE2_INTRO:
        b->isWarning = false;
        b->visualHp += (BOSS_MAX_HP / BOSS_INTRO_TIME) * deltaTime;
        if (b->visualHp >= BOSS_MAX_HP) {
            b->visualHp = (float)BOSS_MAX_HP;
            b->hp = BOSS_MAX_HP;
            b->isInvincible = false;
            b->state = B_PHASE2_MAIN;
        }
        break;

    case B_PHASE2_MAIN:
        b->isWarning = false;
        Pattern_SwayRoad(b, deltaTime);
        if (b->hp <= 0) b->state = B_DEAD;
        break;
    }

    int currentSize = (int)(BOSS_SIZE * b->sizeScale);
    if (currentSize <= 0 && b->sizeScale > 0.1f) currentSize = BOSS_SIZE;
    b->drawRect.w = currentSize;
    b->drawRect.h = currentSize;
    b->drawRect.x = (int)(b->x + (BOSS_SIZE - currentSize) / 2);
    b->drawRect.y = (int)(b->y + (BOSS_SIZE - currentSize) / 2);
}

//보스 출력 코드
void DrawBoss(SDL_Renderer* renderer, BossData* b) {
    if (b->state == B_DEAD) return;
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

    if (b->sizeScale > 0.001f && gBossTexture != nullptr) {
        Uint8 r = 255, g = 255, bl = 255;
        if (b->hitTimer > 0) { r = 255; g = 100; bl = 100; }
        else if (b->state == B_MELEE_ATTACK) { r = 255; g = 50; bl = 50; }
        else if (b->isWarning) {
            float flash = (sinf(SDL_GetTicks() * 0.015f) + 1.0f) / 2.0f;
            r = 255; g = 200 + (Uint8)(flash * 55); bl = 0;
        }
        else if (b->state == B_PHASE_TRANSITION) { r = 255; g = 255; bl = 0; }

        SDL_SetTextureColorMod(gBossTexture, r, g, bl);
        SDL_RenderCopyEx(renderer, gBossTexture, NULL, &b->drawRect, (double)b->rotation, NULL, SDL_FLIP_NONE);
        SDL_SetTextureColorMod(gBossTexture, 255, 255, 255);
    }

    float displayHp = (b->visualHp < 0) ? 0 : b->visualHp;
    float hpRatio = displayHp / (float)BOSS_MAX_HP;
    SDL_Rect hpBg = { 100, 30, SCREEN_WIDTH - 200, 20 };
    SDL_Rect hpFill = { 100, 30, (int)((SCREEN_WIDTH - 200) * hpRatio), 20 };
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &hpBg);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &hpFill);
}
