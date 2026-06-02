#include "Projectile.hpp"
#include "MapSystem.hpp"
#include "Imagemanager.hpp"
#include <math.h>

Projectile bullets[MAX_PROJECTILES];
static Uint32 lastFireTime = 0;

//투사체 초기화
void InitProjectiles() {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        bullets[i].active = false;
    }
}

//플레이어용 투사체 발사 처리 함수
void FireProjectile(float startX, float startY, float deltaTime) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastFireTime < (Uint32)FIRE_DELAY) return; //투사체 발사 딜레이 적용

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY); //마우스 위치로

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 0; //발사대상
            bullets[i].type = 0; //특수기믹(초록 투사체) x
            bullets[i].angle = 0.0f;

            bullets[i].x = startX + (PLAYER_SIZE / 2.0f) - (PROJECTILE_SIZE / 2.0f);
            bullets[i].y = startY + (PLAYER_SIZE / 2.0f) - (PROJECTILE_SIZE / 2.0f);

            float diffX = (float)mouseX - bullets[i].x;
            float diffY = (float)mouseY - bullets[i].y;
            float distance = sqrtf(diffX * diffX + diffY * diffY);

            if (distance < 0.1f) distance = 0.1f;

            bullets[i].dirX = diffX / distance;
            bullets[i].dirY = diffY / distance;

            lastFireTime = currentTime;
            break;
        }
    }
}

//몹 투사체 발사 처리 함수 (발사 위치(플레이어좌표), 발사 대상(owner)외 동일
void FireEnemyProjectile(float startX, float startY, float targetX, float targetY) {
    static int bossTexToggle = 0;
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 1;   //발사 대상
            bullets[i].type = 0;
            bullets[i].angle = 0.0f;
            bullets[i].texIndex = bossTexToggle;
            bossTexToggle = 1 - bossTexToggle;

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

//보스용 3페이즈 기믹 투사체 발사 함수
void FireProjectile_PHASE3(float startX, float startY, float targetX, float targetY, float speed) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 1;
            bullets[i].type = 1; //초록색 대형 투사체 타입 지정
            bullets[i].angle = 0.0f;

            bullets[i].x = startX;
            bullets[i].y = startY;

            float diffX = targetX - startX;
            float diffY = targetY - startY;
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

//투사체 업데이트
void UpdateAndDrawProjectiles(SDL_Renderer* renderer, float deltaTime) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (bullets[i].active) {
            //타입 및 발사 주체에 따른 이동 속도 차등 적용
            float currentSpeed;
            if (bullets[i].type == 1) currentSpeed = 150.0f;
            else if (bullets[i].owner == 1) currentSpeed = BOSS_PROJECTILE_SPEED;
            else currentSpeed = PROJECTILE_SPEED;
            int currentSize = (bullets[i].type == 1) ? (PROJECTILE_SIZE * 2) : PROJECTILE_SIZE;

            bullets[i].x += bullets[i].dirX * currentSpeed * deltaTime;
            bullets[i].y += bullets[i].dirY * currentSpeed * deltaTime;

            //화면을 아예 벗어나거나 벽에 부딪히면 소멸
            float margin = (bullets[i].type == 1) ? 50.0f : 0.0f;

            //화면 이탈 검사
            if (bullets[i].x < -margin || bullets[i].x > SCREEN_WIDTH + margin ||
                bullets[i].y < -margin || bullets[i].y > SCREEN_HEIGHT + margin) {
                bullets[i].active = false;
                continue;
            }

            //벽 충돌 검사(단 초록투사체는 제외)
            if (bullets[i].type != 1 && IsWall(bullets[i].x, bullets[i].y)) {
                bullets[i].active = false;
                continue;
            }

            bullets[i].angle += PROJECTILE_SPIN_SPEED * deltaTime;
            if (bullets[i].angle >= 360.0f) bullets[i].angle -= 360.0f;

            SDL_Rect bRect = { (int)bullets[i].x, (int)bullets[i].y, currentSize, currentSize };
            double angle = (double)bullets[i].angle;

            if (bullets[i].type == 1) { //초록 투사체
                if (gEnemyProjectileTexture != nullptr) {
                    SDL_SetTextureColorMod(gEnemyProjectileTexture, 0, 255, 0);
                    SDL_RenderCopyEx(renderer, gEnemyProjectileTexture, NULL, &bRect, angle, NULL, SDL_FLIP_NONE);
                    SDL_SetTextureColorMod(gEnemyProjectileTexture, 255, 255, 255);
                }
                else {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                    SDL_RenderFillRect(renderer, &bRect);
                }
            }
            else {
                SDL_Texture* targetTexture = nullptr;
                if (bullets[i].owner == 0) {
                    targetTexture = gProjectileTexture;
                }
                else {
                    //보스 투사체: texIndex에 따라 두 이미지 교대 사용
                    targetTexture = (bullets[i].texIndex == 0) ? gBossProjectileTex1 : gBossProjectileTex2;
                    if (targetTexture == nullptr) targetTexture = gEnemyProjectileTexture;
                }
                if (targetTexture != nullptr) {
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
}
