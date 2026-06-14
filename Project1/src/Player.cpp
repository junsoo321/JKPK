#include "Player.hpp"
#include "MapSystem.hpp"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Boss.hpp"
#include "Enemy.hpp"
#include <iostream>

void InitPlayer(PlayerData* p)
{
    p->x = SCREEN_WIDTH / 2.0f;
    p->y = SCREEN_HEIGHT / 2.0f;
    p->speed = PLAYER_SPEED;
    p->hp = PLAYER_HP;
    p->isInvincible = false;
    p->invincibleEndTime = 0;
}

//키보드 입력 기반 이동, 4꼭짓점 벽 충돌, 방 클리어 시 상하좌우 개방 구역 이동 판정
void UpdatePlayer(PlayerData* p, const Uint8* keyboardState, float deltaTime)
{
    Uint32 now = SDL_GetTicks();

    if (p->isInvincible && now > p->invincibleEndTime) {
        p->isInvincible = false;
    }

    float nextX = p->x;
    float nextY = p->y;
    float move = p->speed * deltaTime;

    if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP])    nextY -= move;
    if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN])  nextY += move;
    if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT])  nextX -= move;
    if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) nextX += move;

    // 플레이어 히트박스(가로세로 28px 기준) 4모서리 벽 충돌 스크리닝
    bool canMove = true;
    if (IsWall(nextX, nextY) || IsWall(nextX + 28, nextY) || IsWall(nextX, nextY + 28) || IsWall(nextX + 28, nextY + 28)) {
        canMove = false;
    }

    if (canMove) {
        p->x = nextX;
        p->y = nextY;
    }

    // 현재 방에 몹이 남아있다면 문이 가로막혀 방 이동 불가
    if (AreEnemiesAlive()) return;

    // 우측 방 이동
    if (p->x > SCREEN_WIDTH - 35) {
        if (currentRoom->right && currentRoom->right->exists) {
            MoveToNextRoom(3);
            p->x = 50;
        }
    }
    // 좌측 방 이동
    else if (p->x < 5) {
        if (currentRoom->left && currentRoom->left->exists) {
            MoveToNextRoom(2);
            p->x = SCREEN_WIDTH - 80;
        }
    }
    // 상단 방 이동
    else if (p->y < 5) {
        if (currentRoom->up && currentRoom->up->exists) {
            MoveToNextRoom(0);
            p->y = SCREEN_HEIGHT - 80;
        }
    }
    // 하단 방 이동
    else if (p->y > SCREEN_HEIGHT - 35) {
        if (currentRoom->down && currentRoom->down->exists) {
            MoveToNextRoom(1);
            p->y = 50;
        }
    }

    p->drawRect.x = (int)p->x;
    p->drawRect.y = (int)p->y;
    p->drawRect.w = PLAYER_SIZE;
    p->drawRect.h = PLAYER_SIZE;
}

//무적(피격) 타임라인 동안 100ms 간격 프레임 탈락 방식으로 캐릭터 깜빡임 연출
void DrawPlayer(SDL_Renderer* renderer, PlayerData* p)
{
    Uint32 currentTime = SDL_GetTicks();

    if (p->isInvincible) {
        if (currentTime < p->invincibleEndTime) {
            if ((currentTime / 100) % 2 == 0) return; // 프레임 생략으로 깜빡임 구현
        }
        else {
            p->isInvincible = false;
        }
    }

    SDL_Rect playerRect = { (int)p->x, (int)p->y, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderCopy(renderer, gPlayerTexture, NULL, &playerRect);
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

            if (p->isInvincible) return false;

            p->hp -= ENEMY_ATK;
            p->isInvincible = true;
            p->invincibleEndTime = SDL_GetTicks() + 2000; // 2초간 무적 유효

            std::cout << "Player HP : " << (p->hp) << std::endl;

            localBullets[b].active = false;
            if (p->hp <= 0) return true; // 사망 상태 반환
        }
    }
    return false;
}
