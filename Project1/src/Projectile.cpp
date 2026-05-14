#include "Projectile.hpp"
#include "MapSystem.h"
#include "Imagemanager.hpp"
#include <math.h>

Projectile bullets[MAX_PROJECTILES];
static Uint32 lastFireTime = 0;

void InitProjectiles() {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        bullets[i].active = false;
    }
}

void FireProjectile(float startX, float startY, float deltaTime) {
    Uint32 currentTime = SDL_GetTicks();

    // [시간 기반] FIRE_DELAY가 '초' 단위라면 (Uint32)(FIRE_DELAY * 1000.0f)로 비교
    // 만약 Constants.h에 FIRE_DELAY가 ms 단위(예: 150)라면 그대로 사용
    if (currentTime - lastFireTime < (Uint32)FIRE_DELAY) return;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 0;

            bullets[i].x = startX + (PLAYER_SIZE / 2.0f) - (PROJECTILE_SIZE / 2.0f);
            bullets[i].y = startY + (PLAYER_SIZE / 2.0f) - (PROJECTILE_SIZE / 2.0f);

            float diffX = (float)mouseX - bullets[i].x;
            float diffY = (float)mouseY - bullets[i].y;
            float distance = sqrtf(diffX * diffX + diffY * diffY);

            if (distance < 0.1f) distance = 0.1f; // 0 나누기 방지

            bullets[i].dirX = diffX / distance;
            bullets[i].dirY = diffY / distance;

            lastFireTime = currentTime;
            break;
        }
    }
}

void FireEnemyProjectile(float startX, float startY, float targetX, float targetY) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 1;

            bullets[i].x = startX + (ENEMY_SIZE / 2.0f) - (PROJECTILE_SIZE / 2.0f);
            bullets[i].y = startY + (ENEMY_SIZE / 2.0f) - (PROJECTILE_SIZE / 2.0f);

            float diffX = targetX - bullets[i].x;
            float diffY = targetY - bullets[i].y;
            float distance = sqrtf(diffX * diffX + diffY * diffY);

            if (distance < 0.1f) {
                bullets[i].dirX = 0;
                bullets[i].dirY = 1;
            }
            else {
                bullets[i].dirX = diffX / distance;
                bullets[i].dirY = diffY / distance;
            }
            break;
        }
    }
}

void UpdateAndDrawProjectiles(SDL_Renderer* renderer, float deltaTime) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (bullets[i].active) {
            // [핵심 수정] 초당 이동 거리(PROJECTILE_SPEED)에 deltaTime을 곱함
            // 이제 PROJECTILE_SPEED가 500.0f라면 1초에 500픽셀을 이동합니다.
            bullets[i].x += bullets[i].dirX * PROJECTILE_SPEED * deltaTime;
            bullets[i].y += bullets[i].dirY * PROJECTILE_SPEED * deltaTime;

            // 맵 경계 및 벽 충돌 처리
            if (bullets[i].x < 0 || bullets[i].x > SCREEN_WIDTH ||
                bullets[i].y < 0 || bullets[i].y > SCREEN_HEIGHT ||
                IsWall(bullets[i].x, bullets[i].y)) {
                bullets[i].active = false;
                continue;
            }

            SDL_Rect bRect = { (int)bullets[i].x, (int)bullets[i].y, PROJECTILE_SIZE, PROJECTILE_SIZE };
            SDL_Texture* targetTexture = (bullets[i].owner == 0) ? gProjectileTexture : gEnemyProjectileTexture;

            if (targetTexture != nullptr) {
                double angle = atan2(bullets[i].dirY, bullets[i].dirX) * (180.0 / M_PI);
                SDL_RenderCopyEx(renderer, targetTexture, NULL, &bRect, angle, NULL, SDL_FLIP_NONE);
            }
            else {
                if (bullets[i].owner == 0) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                else SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(renderer, &bRect);
            }
        }
    }
}
