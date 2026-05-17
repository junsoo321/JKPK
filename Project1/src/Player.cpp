#include "Player.hpp"
#include "MapSystem.h"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Boss.hpp"

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

    //키 입력: 키를 누르면 즉시 최대 속도, 떼면 마찰로 서서히 감속
    float friction = 1.0f - PLAYER_FRICTION * deltaTime;
    if (friction < 0.0f) friction = 0.0f;

    float targetVx = 0.0f, targetVy = 0.0f;
    if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP])    targetVy -= p->speed;
    if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN])  targetVy += p->speed;
    if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT])  targetVx -= p->speed;
    if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) targetVx += p->speed;

    p->vx = (targetVx != 0.0f) ? targetVx : p->vx * friction;
    p->vy = (targetVy != 0.0f) ? targetVy : p->vy * friction;

    float nextX = p->x + p->vx * deltaTime;
    float nextY = p->y + p->vy * deltaTime;

    // 충돌 맵의 yellow(type 3) 타일 기준으로 각 방향 문 통과 가능 여부 확인
    float dFootTop = p->y + PLAYER_FOOT_OFFSET;
    float dFootBot = p->y + PLAYER_SIZE - 2;
    bool canGoLeft  = HasDoorAtEdge(2, dFootTop, dFootBot) && currentRoomX > 0;
    bool canGoRight = HasDoorAtEdge(3, dFootTop, dFootBot) && currentRoomX < MAX_ROOMS_X - 1;
    bool canGoUp    = HasDoorAtEdge(0, p->x, p->x + PLAYER_SIZE - 2) && currentRoomY > 0;
    bool canGoDown  = HasDoorAtEdge(1, p->x, p->x + PLAYER_SIZE - 2) && currentRoomY < MAX_ROOMS_Y - 1;

    //맵 이동 판정 — 발 히트박스가 문 타일에 닿으면 즉시 이동
    const int EDGE = 5;
    const int RIGHT_TRIGGER  = SCREEN_WIDTH  - PLAYER_SIZE - EDGE;
    const int BOTTOM_TRIGGER = SCREEN_HEIGHT - PLAYER_SIZE - EDGE;

    float nFootTop = nextY + PLAYER_FOOT_OFFSET;
    float nFootBot = nextY + PLAYER_SIZE - 2;
    float nFootL   = nextX;
    float nFootR   = nextX + PLAYER_SIZE - 2;

    // 문 타일 깊이(0~6 tiles)보다 한 칸 더 안쪽에 스폰하여 재전환 방지
    const int SPAWN_INSET = 7 * TILE_SIZE;

    if (canGoLeft  && IsTouchingEdgeDoor(2, nFootL, nFootR, nFootTop, nFootBot)) {
        MoveToNextRoom(2);
        int cy = GetDoorCenter(3);
        p->x = (float)(SCREEN_WIDTH - PLAYER_SIZE - SPAWN_INSET);
        p->y = (cy >= 0) ? cy - PLAYER_SIZE / 2.0f : SCREEN_HEIGHT / 2.0f - PLAYER_SIZE / 2.0f;
        p->vx = 0.0f; p->vy = 0.0f; return;
    }
    if (canGoRight && IsTouchingEdgeDoor(3, nFootL, nFootR, nFootTop, nFootBot)) {
        MoveToNextRoom(3);
        int cy = GetDoorCenter(2);
        p->x = (float)SPAWN_INSET;
        p->y = (cy >= 0) ? cy - PLAYER_SIZE / 2.0f : SCREEN_HEIGHT / 2.0f - PLAYER_SIZE / 2.0f;
        p->vx = 0.0f; p->vy = 0.0f; return;
    }
    if (canGoUp    && IsTouchingEdgeDoor(0, nFootL, nFootR, nFootTop, nFootBot)) {
        MoveToNextRoom(0);
        int cx = GetDoorCenter(1);
        p->x = (cx >= 0) ? cx - PLAYER_SIZE / 2.0f : SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
        p->y = (float)(SCREEN_HEIGHT - PLAYER_SIZE - SPAWN_INSET);
        p->vx = 0.0f; p->vy = 0.0f; return;
    }
    if (canGoDown  && IsTouchingEdgeDoor(1, nFootL, nFootR, nFootTop, nFootBot)) {
        MoveToNextRoom(1);
        int cx = GetDoorCenter(0);
        p->x = (cx >= 0) ? cx - PLAYER_SIZE / 2.0f : SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
        p->y = (float)SPAWN_INSET;
        p->vx = 0.0f; p->vy = 0.0f; return;
    }

    //벽 충돌 검사 — 발 히트박스 기준 (X, Y 축 독립 처리 → 벽면 슬라이딩 가능)
    //스프라이트 상단에서 PLAYER_FOOT_OFFSET 아래를 발 상단으로 사용
    float footTop = p->y   + PLAYER_FOOT_OFFSET;
    float footBot = p->y   + PLAYER_SIZE - 2;

    bool blockX = IsWall(nextX,                  footTop) || IsWall(nextX + PLAYER_SIZE - 2, footTop) ||
                  IsWall(nextX,                  footBot)  || IsWall(nextX + PLAYER_SIZE - 2, footBot);
    bool blockY = IsWall(p->x,                   nFootTop) || IsWall(p->x  + PLAYER_SIZE - 2, nFootTop) ||
                  IsWall(p->x,                   nFootBot)  || IsWall(p->x  + PLAYER_SIZE - 2, nFootBot);

    //화면 경계 — 문이 없거나 인접 방이 없으면 이동 불가
    if (nextX < EDGE                && !canGoLeft)  blockX = true;
    if (nextX > RIGHT_TRIGGER       && !canGoRight) blockX = true;
    if (nextY < EDGE                && !canGoUp)    blockY = true;
    if (nextY > BOTTOM_TRIGGER      && !canGoDown)  blockY = true;

    if (!blockX) p->x = nextX; else p->vx = 0.0f;
    if (!blockY) p->y = nextY; else p->vy = 0.0f;

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
