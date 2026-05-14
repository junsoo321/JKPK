#include "Player.hpp"
#include "MapSystem.h"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include <iostream>

void InitPlayer(PlayerData* p) {
    p->x = SCREEN_WIDTH / 2.0f;  //시작 X (화면 중앙)
    p->y = SCREEN_HEIGHT / 2.0f; //시작 Y (화면 중앙)
    p->speed = PLAYER_SPEED;    //이동 속도
    p->hp = PLAYER_HP;          //체력
    p->isInvincible = false;      // 시작할 때는 무적이 아님
    p->invincibleEndTime = 0;     // 종료 시간 초기화
}

//플레이어 이동 함수
void UpdatePlayer(PlayerData* p, const Uint8* keyboardState, float deltaTime) {
    Uint32 now = SDL_GetTicks();

    // 1. 무적 시간 체크 (절대 시간 방식 유지)
    if (p->isInvincible && now > p->invincibleEndTime) {
        p->isInvincible = false;
    }

    float nextX = p->x;
    float nextY = p->y;

    // 2. 키보드 입력 및 이동 거리 계산 (시간 기반)
    // p->speed는 이제 '초당 픽셀 수'(예: 300.0f)여야 합니다.
    float moveDistance = p->speed * deltaTime;

    if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP])    nextY -= moveDistance;
    if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN])  nextY += moveDistance;
    if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT])  nextX -= moveDistance;
    if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) nextX += moveDistance;

    // 3. 맵 이동 판정 (화면 경계 체크)
    // 오른쪽 끝
    if (nextX > SCREEN_WIDTH - 35) {
        if (currentRoomX < MAX_ROOMS_X - 1) {
            MoveToNextRoom(3);
            p->x = 50;
            return;
        }
    }
    // 왼쪽 끝
    else if (nextX < 5) {
        if (currentRoomX > 0) {
            MoveToNextRoom(2);
            p->x = SCREEN_WIDTH - 80;
            return;
        }
    }
    // 위쪽 끝
    else if (nextY < 5) {
        if (currentRoomY > 0) {
            MoveToNextRoom(0);
            p->y = SCREEN_HEIGHT - 80;
            return;
        }
    }
    // 아래쪽 끝
    else if (nextY > SCREEN_HEIGHT - 35) {
        if (currentRoomY < MAX_ROOMS_Y - 1) {
            MoveToNextRoom(1);
            p->y = 50;
            return;
        }
    }

    // 4. 벽 충돌 검사
    // 플레이어의 히트박스 크기(예: 28px)를 고려한 IsWall 체크
    bool canMove = true;
    if (IsWall(nextX, nextY) || IsWall(nextX + 28, nextY) ||
        IsWall(nextX, nextY + 28) || IsWall(nextX + 28, nextY + 28)) {
        canMove = false;
    }

    if (canMove) {
        p->x = nextX;
        p->y = nextY;
    }

    // 5. 렌더링용 사각형 업데이트
    p->drawRect.x = (int)p->x;
    p->drawRect.y = (int)p->y;
    p->drawRect.w = PLAYER_SIZE;
    p->drawRect.h = PLAYER_SIZE;
}

void DrawPlayer(SDL_Renderer* renderer, PlayerData* p) {
    Uint32 currentTime = SDL_GetTicks();

    // 무적 상태일 때 깜빡임 처리
    if (p->isInvincible) {
        // 현재 시간이 무적 종료 시간보다 작을 때만 깜빡임
        if (currentTime < p->invincibleEndTime) {
            // 100ms 단위로 껐다 켰다 함 (나머지 연산 이용)
            if ((currentTime / 100) % 2 == 0) {
                return; // 이번 프레임은 그리지 않고 건너뜀 (깜빡임 효과)
            }
        }
        else {
            // 무적 시간이 끝났으면 플래그 해제
            p->isInvincible = false;
        }
    }

    // 플레이어 그리기 (기존 코드)
    SDL_Rect playerRect = { (int)p->x, (int)p->y, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderCopy(renderer, gPlayerTexture, NULL, &playerRect);
}

bool CheckCollision(PlayerData* p, void* bulletArray) {
    // 무적 상태라면 충돌 계산 자체를 건너뜀
    if (p->isInvincible) return false;

    Projectile* bullets = (Projectile*)bulletArray;
    for (int b = 0; b < MAX_PROJECTILES; b++) {
        if (!bullets[b].active) continue;
        if (bullets[b].owner == 0) continue;

        if (bullets[b].x < p->x + PLAYER_SIZE && bullets[b].x + PROJECTILE_SIZE > p->x &&
            bullets[b].y < p->y + PLAYER_SIZE && bullets[b].y + PROJECTILE_SIZE > p->y) {

            p->hp -= ENEMY_ATK;

            // 피격 시 무적 설정 (2초)
            p->isInvincible = true;
            p->invincibleEndTime = SDL_GetTicks() + 2000;

            std::cout << "Player HP : " << (p->hp) << std::endl;

            bullets[b].active = false;
            if (p->hp <= 0) return true;
        }
    }
    return false;
}
