#include "Player.hpp"
#include "MapSystem.hpp"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Boss.hpp"
#include "Enemy.h"
#include <iostream>
#include <cmath>
#include <cstdio>

// 플레이어 히트박스 4코너가 모두 벽 밖인지 확인
static bool IsSafePosition(float x, float y) {
    return !IsWall(x,      y     ) &&
           !IsWall(x + 28, y     ) &&
           !IsWall(x,      y + 28) &&
           !IsWall(x + 28, y + 28);
}

// 좌우 방 이동 시 스폰 Y 보정: spawnX를 고정하고 preferredY에서 가장 가까운 안전한 Y 탐색
static float SafeSpawnY(float spawnX, float preferredY) {
    if (IsSafePosition(spawnX, preferredY)) return preferredY;
    for (int d = TILE_SIZE; d < SCREEN_HEIGHT; d += TILE_SIZE) {
        if (preferredY - d >= 0                  && IsSafePosition(spawnX, preferredY - d)) return preferredY - d;
        if (preferredY + d + 28 <= SCREEN_HEIGHT && IsSafePosition(spawnX, preferredY + d)) return preferredY + d;
    }
    return SCREEN_HEIGHT / 2.0f;
}

// 상하 방 이동 시 스폰 X 보정: spawnY를 고정하고 preferredX에서 가장 가까운 안전한 X 탐색
static float SafeSpawnX(float preferredX, float spawnY) {
    if (IsSafePosition(preferredX, spawnY)) return preferredX;
    for (int d = TILE_SIZE; d < SCREEN_WIDTH; d += TILE_SIZE) {
        if (preferredX - d >= 0                 && IsSafePosition(preferredX - d, spawnY)) return preferredX - d;
        if (preferredX + d + 28 <= SCREEN_WIDTH && IsSafePosition(preferredX + d, spawnY)) return preferredX + d;
    }
    return SCREEN_WIDTH / 2.0f;
}

// 갇혔을 때 타일 격자 기준으로 가장 가까운 안전 위치 탐색
static bool FindNearestSafePos(float cx, float cy, float* outX, float* outY) {
    int startCol = (int)(cx / TILE_SIZE);
    int startRow = (int)(cy / TILE_SIZE);
    for (int radius = 1; radius <= 60; radius++) {
        for (int dc = -radius; dc <= radius; dc++) {
            for (int dr = -radius; dr <= radius; dr++) {
                if (abs(dc) != radius && abs(dr) != radius) continue;
                float nx = (startCol + dc) * (float)TILE_SIZE;
                float ny = (startRow + dr) * (float)TILE_SIZE;
                if (nx < 0 || ny < 0 || nx + 28 > SCREEN_WIDTH || ny + 28 > SCREEN_HEIGHT) continue;
                if (IsSafePosition(nx, ny)) { *outX = nx; *outY = ny; return true; }
            }
        }
    }
    return false;
}

void InitPlayer(PlayerData* p) {
    p->x = SCREEN_WIDTH / 2.0f;
    p->y = SCREEN_HEIGHT / 2.0f;
    p->vx = 0.0f;
    p->vy = 0.0f;
    p->speed = PLAYER_SPEED;
    p->hp = PLAYER_HP;
    p->isInvincible = false;
    p->invincibleEndTime = 0;
    p->animDir    = 0;
    p->facingRight = true;
    p->animFrame  = 0;
    p->animTimer  = 0.0f;
    p->hurtTimer  = 0.0f;
    p->isDead     = false;
}

//플레이어 이동 함수
void UpdatePlayer(PlayerData* p, const Uint8* keyboardState, float deltaTime) {
    Uint32 now = SDL_GetTicks();

    //무적 확인
    if (p->isInvincible && now > p->invincibleEndTime) {
        p->isInvincible = false;
    }

    // 벽에 갇힌 경우 감지 및 R키 탈출
    if (!IsSafePosition(p->x, p->y)) {
        static Uint32 lastWarnTime = 0;
        if (now - lastWarnTime > 1000) {
            printf("[DEBUG] 플레이어가 벽에 갇혔습니다. (%.0f, %.0f) - R키를 눌러 탈출\n", p->x, p->y);
            lastWarnTime = now;
        }
        if (keyboardState[SDL_SCANCODE_R]) {
            float sx, sy;
            if (FindNearestSafePos(p->x, p->y, &sx, &sy)) {
                printf("[DEBUG] 탈출 성공: (%.0f, %.0f)\n", sx, sy);
                p->x = sx;
                p->y = sy;
            }
        }
    }

    //키 입력: 키를 누르면 즉시 최대 속도, 떼면 마찰로 서서히 감속
    float friction = 1.0f - PLAYER_FRICTION * deltaTime;
    if (friction < 0.0f) friction = 0.0f;

    float move  = p->speed * deltaTime;
    float nextX = p->x;
    float nextY = p->y;
    p->vx = 0.0f;
    p->vy = 0.0f;

    //키보드 입력 받기, 이동할 좌표 저장
    if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP])    { nextY -= move; p->vy = -p->speed; }
    if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN])  { nextY += move; p->vy =  p->speed; }
    if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT])  { nextX -= move; p->vx = -p->speed; }
    if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) { nextX += move; p->vx =  p->speed; }

    //벽 충돌 검사
    bool canMove = true;
    if (IsWall(nextX, nextY) || IsWall(nextX + 28, nextY) || IsWall(nextX, nextY + 28) || IsWall(nextX + 28, nextY + 28)) {
        canMove = false;
    }

    if (canMove) {
        p->x = nextX;
        p->y = nextY;
    }

    // 방 이동 처리
    if (AreEnemiesAlive())
    {
        return;
    }
// 오른쪽
    if (p->x > SCREEN_WIDTH - 35) {
        if (currentRoom->right && currentRoom->right->exists) {
            MoveToNextRoom(3);
            p->x = 50;
            p->y = SafeSpawnY(p->x, p->y);
        }
    }

    // 왼쪽
    else if (p->x < 5) {
        if (currentRoom->left && currentRoom->left->exists) {
            MoveToNextRoom(2);
            p->x = SCREEN_WIDTH - 80;
            p->y = SafeSpawnY(p->x, p->y);
        }
    }

    // 위
    else if (p->y < 5) {
        if (currentRoom->up && currentRoom->up->exists) {
            MoveToNextRoom(0);
            p->y = SCREEN_HEIGHT - 80;
            p->x = SafeSpawnX(p->x, p->y);
        }
    }

    // 아래
    else if (p->y > SCREEN_HEIGHT - 35) {
        if (currentRoom->down && currentRoom->down->exists) {
            MoveToNextRoom(1);
            p->y = 50;
            p->x = SafeSpawnX(p->x, p->y);
        }
    }

    //렌더링용 사각형 업데이트
    p->drawRect.x = (int)p->x;
    p->drawRect.y = (int)p->y;
    p->drawRect.w = PLAYER_SIZE;
    p->drawRect.h = PLAYER_SIZE;

    // 이동 방향 감지
    bool moving = (fabsf(p->vx) > 20.0f || fabsf(p->vy) > 20.0f);
    if (fabsf(p->vx) > fabsf(p->vy)) {
        p->animDir = 2;
        if (p->vx > 20.0f)       p->facingRight = true;
        else if (p->vx < -20.0f) p->facingRight = false;
    } else if (p->vy < -20.0f) {
        p->animDir = 1;
    } else if (moving) {
        p->animDir = 0;
    }

    // hurt 타이머 감소
    if (p->hurtTimer > 0.0f) p->hurtTimer -= deltaTime;

    // 현재 상태에 따른 프레임 수 / 속도
    int   maxFrames;
    float frameTime;
    if (p->isDead)              { maxFrames = 3; frameTime = 0.15f; }
    else if (p->hurtTimer > 0.0f) { maxFrames = 2; frameTime = 0.1f;  }
    else if (moving)            { maxFrames = 4; frameTime = 0.1f;  }
    else                        { maxFrames = 2; frameTime = 0.35f; }

    p->animTimer += deltaTime;
    if (p->animTimer >= frameTime) {
        p->animTimer = 0.0f;
        if (p->isDead)
            p->animFrame = (p->animFrame < maxFrames - 1) ? p->animFrame + 1 : maxFrames - 1;
        else
            p->animFrame = (p->animFrame + 1) % maxFrames;
    }
    // 상태가 바뀌면 프레임 범위 초과 방지
    if (p->animFrame >= maxFrames) p->animFrame = 0;
}

void DrawPlayer(SDL_Renderer* renderer, PlayerData* p) {
    Uint32 now = SDL_GetTicks();

    // 무적 깜빡임
    if (p->isInvincible) {
        if (now < p->invincibleEndTime) {
            if ((now / 100) % 2 == 0) return;
        } else {
            p->isInvincible = false;
        }
    }

    // 텍스처 선택
    SDL_Texture* tex;
    if      (p->isDead)              tex = gPlayerDeathTex;
    else if (p->hurtTimer > 0.0f)    tex = gPlayerHurtTex;
    else if (fabsf(p->vx) > 20.0f || fabsf(p->vy) > 20.0f) tex = gPlayerWalkTex;
    else                             tex = gPlayerIdleTex;

    if (!tex) return;

    // 스프라이트 시트에서 현재 프레임 잘라내기
    SDL_Rect src = {
        p->animFrame * CHAR_FRAME_SIZE,
        p->animDir   * CHAR_FRAME_SIZE,
        CHAR_FRAME_SIZE,
        CHAR_FRAME_SIZE
    };
    // 히트박스(p->x, p->y)에서 오프셋을 빼서 스프라이트의 실제 도트가 히트박스와 정렬되도록 렌더링
    SDL_Rect dst = {
        (int)p->x - PLAYER_RENDER_OFFSET_X,
        (int)p->y - PLAYER_RENDER_OFFSET_Y,
        CHAR_FRAME_SIZE * 3,
        CHAR_FRAME_SIZE * 3
    };

    // 왼쪽 방향이면 수평 반전
    SDL_RendererFlip flip = (p->animDir == 2 && !p->facingRight)
                          ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, tex, &src, &dst, 0.0, NULL, flip);
}

//화면 왼쪽 위에 하트로 HP 표시
void DrawHearts(SDL_Renderer* renderer, PlayerData* p) {
    const int HEART_SIZE    = 32;
    const int HEART_SPACING = 4;
    const int START_X       = 10;
    const int START_Y       = 10;
    const int TOTAL_HEARTS  = 5;   // PLAYER_HP(100) / 20
    const int HP_PER_HEART  = 20;
    const int HP_PER_HALF   = 10;

    // 피격 직후 깜빡임: hurtTimer 남아있는 동안 100ms 주기로 토글
    if (p->hurtTimer > 0.0f && (SDL_GetTicks() / 100) % 2 == 0) return;

    for (int i = 0; i < TOTAL_HEARTS; i++) {
        SDL_Texture* tex;
        if      (p->hp >= (i + 1) * HP_PER_HEART) tex = gHeartFullTex;
        else if (p->hp >= i * HP_PER_HEART + HP_PER_HALF) tex = gHeartHalfTex;
        else                                        tex = gHeartEmptyTex;

        if (!tex) continue;
        SDL_Rect dst = { START_X + i * (HEART_SIZE + HEART_SPACING), START_Y, HEART_SIZE, HEART_SIZE };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }
}

//플레이어 피격(투사체 피격) 검사 및 처리 함수
bool CheckCollision(PlayerData* p, void* bulletArray, void* bossData) {

    Projectile* bullets = (Projectile*)bulletArray;
    BossData* boss = (BossData*)bossData;
    for (int b = 0; b < MAX_PROJECTILES; b++) {
        if (!bullets[b].active) continue;
        if (bullets[b].owner == 0) continue;

        if (bullets[b].x < p->x + PLAYER_SIZE && bullets[b].x + PROJECTILE_SIZE > p->x &&
            bullets[b].y < p->y + PLAYER_SIZE && bullets[b].y + PROJECTILE_SIZE > p->y) {

            //3페이즈 초록 투사체 플레이어 피격 처리(아래 무적보다 먼저 확인해서, 무적과 상관없이 작용)
            if (bullets[b].type == 1) {
                bullets[b].active = false; //투사체 소멸
                boss->greenBulletCount--;        //보스의 남은 투사체 개수 감소
                continue;                     //플레이어는 데미지를 입지 않고 다음 투사체 검사로 넘어감
            }
            if (p->isInvincible) { //플레이어 무적일 경우, 투사체 검사 X
                return false;
            }
            p->hp -= ENEMY_ATK;
            p->hurtTimer = 0.3f;

            p->isInvincible = true;
            p->invincibleEndTime = SDL_GetTicks() + 2000;

            bullets[b].active = false;
            if (p->hp <= 0) { p->isDead = true; return true; }
        }
    }
    return false;
}
