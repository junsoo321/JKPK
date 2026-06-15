#include "Player.hpp"
#include "MapSystem.hpp"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Boss.hpp"
#include "Enemy.hpp"
#include <iostream>
#include <cmath>
#include <cstdio>

// 플레이어 히트박스(28x28)가 문 rect와 겹치는지 AABB 체크
static bool TouchesDoor(float px, float py, SDL_Rect door) {
    if (door.w == 0 || door.h == 0) return false;
    return px      < door.x + door.w &&
           px + 28 > door.x          &&
           py      < door.y + door.h &&
           py + 28 > door.y;
}

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
    p->projectileSpeedMult = 1.0f;
    p->hp = PLAYER_HP;
    p->isInvincible = false;
    p->invincibleEndTime = 0;
    p->animDir    = 0;
    p->facingRight = true;
    p->animFrame  = 0;
    p->animTimer  = 0.0f;
    p->hurtTimer  = 0.0f;
    p->healTimer  = 0.0f;
    p->isDead     = false;
    p->itemCount  = 0;
    for (int i = 0; i < MAX_INVENTORY; i++) p->collectedItems[i] = -1;
}

//키보드 입력 기반 이동, 4꼭짓점 벽 충돌, 방 클리어 시 상하좌우 개방 구역 이동 판정
void UpdatePlayer(PlayerData* p, const Uint8* keyboardState, float deltaTime)
{
    Uint32 now = SDL_GetTicks();

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

    // 플레이어 히트박스(가로세로 28px 기준) 4모서리 벽 충돌 스크리닝
    bool canMove = true;
    if (IsWall(nextX, nextY) || IsWall(nextX + 28, nextY) || IsWall(nextX, nextY + 28) || IsWall(nextX + 28, nextY + 28)) {
        canMove = false;
    }

    // 보상 탁자 충돌 — rewardAvailable 동안 통행 불가
    if (canMove && currentRoom && currentRoom->rewardAvailable) {
        SDL_Rect nextR  = { (int)nextX, (int)nextY, PLAYER_SIZE, PLAYER_SIZE };
        SDL_Rect tableR = { SCREEN_WIDTH / 2 - 37, SCREEN_HEIGHT / 2 - 37, 75, 75 };
        if (SDL_HasIntersection(&nextR, &tableR)) {
            canMove = false;
        }
    }

    if (canMove) {
        p->x = nextX;
        p->y = nextY;
    }

    // 히트박스 갱신
    p->drawRect.x = (int)p->x;
    p->drawRect.y = (int)p->y;
    p->drawRect.w = PLAYER_SIZE;
    p->drawRect.h = PLAYER_SIZE;

    // 이동 방향 감지 (항상 실행 — 몹 유무와 무관)
    bool moving = (fabsf(p->vx) > 20.0f || fabsf(p->vy) > 20.0f);
    if (fabsf(p->vx) > fabsf(p->vy)) {
        p->animDir = 1;  // row1: 오른쪽 옆모습 (좌이동 시 flip)
        if (p->vx > 20.0f)       p->facingRight = true;
        else if (p->vx < -20.0f) p->facingRight = false;
    } else if (p->vy < -20.0f) {
        p->animDir = 2;  // row2: 뒷모습(위)
    } else if (moving) {
        p->animDir = 0;  // row0: 앞모습(아래)
    }

    // hurt / heal 타이머 감소 (항상 실행)
    if (p->hurtTimer > 0.0f) p->hurtTimer -= deltaTime;
    if (p->healTimer > 0.0f) p->healTimer -= deltaTime;

    // 애니메이션 프레임 갱신 (항상 실행)
    {
        int   maxFrames;
        float frameTime;
        if (p->isDead)                { maxFrames = 3; frameTime = 0.15f; }
        else if (p->hurtTimer > 0.0f) { maxFrames = 2; frameTime = 0.1f;  }
        else if (moving)              { maxFrames = 4; frameTime = 0.1f;  }
        else                          { maxFrames = 2; frameTime = 0.35f; }

        p->animTimer += deltaTime;
        if (p->animTimer >= frameTime) {
            p->animTimer = 0.0f;
            if (p->isDead)
                p->animFrame = (p->animFrame < maxFrames - 1) ? p->animFrame + 1 : maxFrames - 1;
            else
                p->animFrame = (p->animFrame + 1) % maxFrames;
        }
        if (p->animFrame >= maxFrames) p->animFrame = 0;
    }

    // 현재 방에 몹이 남아있다면 방 이동 불가 (문 전환만 막음)
    if (AreEnemiesAlive()) return;

    SDL_Rect dU = GetDoorRect(0), dD = GetDoorRect(1);
    SDL_Rect dL = GetDoorRect(2), dR = GetDoorRect(3);

    // 상단 문 → 새 방 하단(D door) 안쪽에 스폰
    if (currentRoom->up && currentRoom->up->exists && TouchesDoor(p->x, p->y, dU)) {
        MoveToNextRoom(0);
        float spawnY = dD.h > 0 ? (float)(dD.y - 32)          : SCREEN_HEIGHT - 80.0f;
        float spawnX = dD.w > 0 ? (float)(dD.x + dD.w / 2 - 14) : p->x;
        p->y = spawnY;
        p->x = SafeSpawnX(spawnX, p->y);
    }
    // 하단 문 → 새 방 상단(U door) 안쪽에 스폰
    else if (currentRoom->down && currentRoom->down->exists && TouchesDoor(p->x, p->y, dD)) {
        MoveToNextRoom(1);
        float spawnY = dU.h > 0 ? (float)(dU.y + dU.h + 4)    : 50.0f;
        float spawnX = dU.w > 0 ? (float)(dU.x + dU.w / 2 - 14) : p->x;
        p->y = spawnY;
        p->x = SafeSpawnX(spawnX, p->y);
    }
    // 좌측 문 → 새 방 우측(R door) 안쪽에 스폰
    else if (currentRoom->left && currentRoom->left->exists && TouchesDoor(p->x, p->y, dL)) {
        MoveToNextRoom(2);
        float spawnX = dR.w > 0 ? (float)(dR.x - 32)            : SCREEN_WIDTH - 80.0f;
        float spawnY = dR.h > 0 ? (float)(dR.y + dR.h / 2 - 14) : p->y;
        p->x = spawnX;
        p->y = SafeSpawnY(p->x, spawnY);
    }
    // 우측 문 → 새 방 좌측(L door) 안쪽에 스폰
    else if (currentRoom->right && currentRoom->right->exists && TouchesDoor(p->x, p->y, dR)) {
        MoveToNextRoom(3);
        float spawnX = dL.w > 0 ? (float)(dL.x + dL.w + 4)      : 50.0f;
        float spawnY = dL.h > 0 ? (float)(dL.y + dL.h / 2 - 14) : p->y;
        p->x = spawnX;
        p->y = SafeSpawnY(p->x, spawnY);
    }
}

//무적(피격) 타임라인 동안 100ms 간격 프레임 탈락 방식으로 캐릭터 깜빡임 연출
void DrawPlayer(SDL_Renderer* renderer, PlayerData* p)
{
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
    SDL_RendererFlip flip = (p->animDir == 1 && !p->facingRight)
                          ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    // 회복 깜빡임: 80ms 주기로 초록 tint
    if (p->healTimer > 0.0f && (now / 80) % 2 == 0)
        SDL_SetTextureColorMod(tex, 80, 255, 80);

    SDL_RenderCopyEx(renderer, tex, &src, &dst, 0.0, NULL, flip);
    SDL_SetTextureColorMod(tex, 255, 255, 255);
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

//적 투사체와의 피격 판정 및 3페이즈 특수 기믹(초록 투사체 상쇄) 처리
bool CheckCollision(PlayerData* p, void* bulletArray, void* bossData)
{
    Projectile* localBullets = (Projectile*)bulletArray;
    BossData* boss = (BossData*)bossData;

    for (int b = 0; b < MAX_PROJECTILES; b++) {
        if (!localBullets[b].active || localBullets[b].owner == 0) continue;

        // AABB 충돌 조건 충족 시
        if (localBullets[b].x < p->x + PLAYER_SIZE && localBullets[b].x + PROJECTILE_SIZE > p->x &&
            localBullets[b].y < p->y + PLAYER_SIZE && localBullets[b].y + PROJECTILE_SIZE > p->y) {

            // 기믹용 초록 투사체(type == 1)는 무적 상태를 무시하고 데미지 없이 흡수 처리
            if (localBullets[b].type == 1) {
                localBullets[b].active = false;
                boss->greenBulletCount--;
                continue;
            }
            if (p->isInvincible) { //플레이어 무적일 경우, 투사체 검사 X
                return false;
            }
            p->hp -= ENEMY_ATK;
            p->hurtTimer = 0.3f;

            p->isInvincible = true;
            p->invincibleEndTime = SDL_GetTicks() + 2000; // 2초간 무적 유효

            localBullets[b].active = false;
            if (p->hp <= 0) { p->isDead = true; return true; } // 사망 상태 반환
        }
    }
    return false;
}
