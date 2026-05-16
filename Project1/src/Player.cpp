#include "Player.hpp"
#include "MapSystem.h"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Boss.hpp"
#include <iostream>

void InitPlayer(PlayerData* p) {
    p->x = SCREEN_WIDTH / 2.0f;  //시작 X (화면 중앙)
    p->y = SCREEN_HEIGHT / 2.0f; //시작 Y (화면 중앙)
    p->speed = PLAYER_SPEED;    //이동 속도
    p->hp = PLAYER_HP;          //체력
    p->isInvincible = false;      //시작시 무적 off
    p->invincibleEndTime = 0;     //종료 시간 초기화
}

//플레이어 이동 함수
void UpdatePlayer(PlayerData* p, const Uint8* keyboardState, float deltaTime) {
    Uint32 now = SDL_GetTicks();

    //무적 확인
    if (p->isInvincible && now > p->invincibleEndTime) {
        p->isInvincible = false;
    }

    float nextX = p->x;
    float nextY = p->y;
    float moveDistance = p->speed * deltaTime;

    //키보드 입력시 이동
    if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP])    nextY -= moveDistance;
    if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN])  nextY += moveDistance;
    if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT])  nextX -= moveDistance;
    if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) nextX += moveDistance;

    //맵 이동 판정 (화면 경계 체크)
    //오른쪽 끝
    if (nextX > SCREEN_WIDTH - 35) {
        if (currentRoomX < MAX_ROOMS_X - 1) {
            MoveToNextRoom(3);
            p->x = 50;
            return;
        }
    }
    //왼쪽 끝
    else if (nextX < 5) {
        if (currentRoomX > 0) {
            MoveToNextRoom(2);
            p->x = SCREEN_WIDTH - 80;
            return;
        }
    }
    //위쪽 끝
    else if (nextY < 5) {
        if (currentRoomY > 0) {
            MoveToNextRoom(0);
            p->y = SCREEN_HEIGHT - 80;
            return;
        }
    }
    //아래쪽 끝
    else if (nextY > SCREEN_HEIGHT - 35) {
        if (currentRoomY < MAX_ROOMS_Y - 1) {
            MoveToNextRoom(1);
            p->y = 50;
            return;
        }
    }

    //벽 충돌 검사
    bool canMove = true;
    if (IsWall(nextX, nextY) || IsWall(nextX + 28, nextY) ||
        IsWall(nextX, nextY + 28) || IsWall(nextX + 28, nextY + 28)) {
        canMove = false;
    }

    if (canMove) {
        p->x = nextX;
        p->y = nextY;
    }

    //렌더링용 사각형 업데이트
    p->drawRect.x = (int)p->x;
    p->drawRect.y = (int)p->y;
    p->drawRect.w = PLAYER_SIZE;
    p->drawRect.h = PLAYER_SIZE;
}

void DrawPlayer(SDL_Renderer* renderer, PlayerData* p) {
    Uint32 currentTime = SDL_GetTicks();

    //무적 상태일 때 깜빡임 처리
    if (p->isInvincible) {
        //현재 시간이 무적 종료 시간보다 작을 때만 깜빡임
        if (currentTime < p->invincibleEndTime) {
            //100ms 단위
            if ((currentTime / 100) % 2 == 0) {
                return;
            }
        }
        else {
            //무적 시간이 끝났으면 플래그 해제
            p->isInvincible = false;
        }
    }

    //플레이어 그리기
    SDL_Rect playerRect = { (int)p->x, (int)p->y, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderCopy(renderer, gPlayerTexture, NULL, &playerRect);
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
            p->hp -= ENEMY_ATK; //데미지

            //피격 시 무적 설정 (2초)
            p->isInvincible = true;
            p->invincibleEndTime = SDL_GetTicks() + 2000;

            std::cout << "Player HP : " << (p->hp) << std::endl;

            bullets[b].active = false;
            if (p->hp <= 0) return true;
        }
    }
    return false;
}
