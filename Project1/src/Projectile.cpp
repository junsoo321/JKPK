#include "Projectile.hpp"
#include "MapSystem.hpp"
#include "ImageManager.hpp"
#include "GameState.hpp"
#include <math.h>

Projectile bullets[MAX_PROJECTILES];
static Uint32 lastFireTime = 0;

void InitProjectiles()
{
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        bullets[i].active = false;
    }
}

// 마우스 좌표 기준 단위 벡터(방향) 연산 후 플레이어 투사체 스폰
void FireProjectile(float startX, float startY, float deltaTime)
{
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastFireTime < (Uint32)FIRE_DELAY) return;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 0; // 플레이어 소유
            bullets[i].type = 0;
            bullets[i].angle = 0.0f;

            // 플레이어 중앙 기준 오프셋 정렬 스폰
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

// 타겟(플레이어) 위치 벡터 방향으로 일반 몹 투사체 스폰
void FireEnemyProjectile(float startX, float startY, float targetX, float targetY)
{
    static int bossTexToggle = 0;
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 1; // 몬스터 소유
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

// 보스 3페이즈용 특수 기믹(흡수 가능한 초록색 대형 투사체, type = 1) 스폰
void FireProjectile_PHASE3(float startX, float startY, float targetX, float targetY, float speed)
{
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 1;
            bullets[i].type = 1;
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

// 퍼즈 상태 스크리닝, 투사체 속도/크기 예외 연산, 화면 외곽 이탈 및 벽 충돌 소멸 판정 처리
void UpdateAndDrawProjectiles(SDL_Renderer* renderer, float deltaTime)
{
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (bullets[i].active) {
            //타입 및 발사 주체에 따른 이동 속도 차등 적용
            float currentSpeed;
            if (bullets[i].type == 1) currentSpeed = 150.0f;
            else if (bullets[i].owner == 1) currentSpeed = BOSS_PROJECTILE_SPEED;
            else currentSpeed = PROJECTILE_SPEED;
            int currentSize = (bullets[i].type == 1) ? (PROJECTILE_SIZE * 2) : PROJECTILE_SIZE;

        // 일시정지가 아닐 때만 프레임 역학 이동 및 충돌 수명 검사 연산 통합 처리
        if (gGameState != GAME_PAUSE) {
            bullets[i].x += bullets[i].dirX * currentSpeed * deltaTime;
            bullets[i].y += bullets[i].dirY * currentSpeed * deltaTime;

            float margin = (bullets[i].type == 1) ? 50.0f : 0.0f;

            // 1. 화면 밖 이탈 스크리닝
            if (bullets[i].x < -margin || bullets[i].x > SCREEN_WIDTH + margin ||
                bullets[i].y < -margin || bullets[i].y > SCREEN_HEIGHT + margin) {
                bullets[i].active = false;
                continue;
            }

            // 2. 일반 투사체 타일맵 벽 충돌 검사 (초록 기믹 구체는 벽 통과)
            if (bullets[i].type != 1 && IsWall(bullets[i].x, bullets[i].y)) {
                bullets[i].active = false;
                continue;
            }
        }

            bullets[i].angle += PROJECTILE_SPIN_SPEED * deltaTime;
            if (bullets[i].angle >= 360.0f) bullets[i].angle -= 360.0f;

        //삼각함수 삼차각 역산 후 회전 복사 플래그 전달
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
