#include "Projectile.hpp"
#include "MapSystem.h" //IsWall
#include "ImageManager.hpp"
#include <math.h>

Projectile bullets[MAX_PROJECTILES];    //투사체 정보 저장 배열
static Uint32 lastFireTime = 0;         //마지막 발사 시간 저장 변수

//투사체 초기화(전부 비활성화)
void InitProjectiles() {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        bullets[i].active = false;
    }
}

//플레이어 투사체 발사 처리 함수
void FireProjectile(float startX, float startY) {
    Uint32 currentTime = SDL_GetTicks(); //현재 시각 저장
    if (currentTime - lastFireTime < FIRE_DELAY) return; //공격속도 조절

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY); //마우스 위치 저장

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true; //투사체 활성화
            bullets[i].owner = 0; //발사 대상 정보(플레이어 : 0)

            //투사체 시작(생성)좌표, 캐릭터 중앙 좌표에서 탄환 크기 절반만큼 보정
            bullets[i].x = startX + (PLAYER_SIZE / 2) - (PROJECTILE_SIZE / 2);
            bullets[i].y = startY + (PLAYER_SIZE / 2) - (PROJECTILE_SIZE / 2);

            //투사체 방향 단위 벡터 계산
            float diffX = (float)mouseX - bullets[i].x;
            float diffY = (float)mouseY - bullets[i].y;
            float distance = sqrtf(diffX * diffX + diffY * diffY);

            //정규화(마우스의 거리에 상관없이 동일한 벡터 계산)
            bullets[i].dirX = diffX / distance;
            bullets[i].dirY = diffY / distance;
            lastFireTime = currentTime; //마지막 발사 시각 저장
            break;
        }
    }
}

//몹 투사체 발사 처리 함수
//이하 플레이어 발사체 처리 함수와 동일
void FireEnemyProjectile(float startX, float startY, float targetX, float targetY) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].owner = 1; //발사 대상 정보(몹 : 1)

            bullets[i].x = startX + (ENEMY_SIZE / 2) - (PROJECTILE_SIZE / 2);
            bullets[i].y = startY + (ENEMY_SIZE / 2) - (PROJECTILE_SIZE / 2);

            float diffX = targetX - bullets[i].x;
            float diffY = targetY - bullets[i].y;
            float distance = sqrtf(diffX * diffX + diffY * diffY);

            bullets[i].dirX = diffX / distance;
            bullets[i].dirY = diffY / distance;
            //몹의 마지막 발사 시간은 투사체가 아닌 몹에서(Enemy.c) 처리
            break;
        }
    }
}

//투사체 출력 및 충돌 판정 체크 함수
void UpdateAndDrawProjectiles(SDL_Renderer* renderer, float deltaTime) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (bullets[i].active) { //활성화된 투사체
            bullets[i].x += bullets[i].dirX * PROJECTILE_SPEED * deltaTime;
            bullets[i].y += bullets[i].dirY * PROJECTILE_SPEED * deltaTime;

            if (bullets[i].x < 0 || bullets[i].x > SCREEN_WIDTH || bullets[i].y < 0 || bullets[i].y > SCREEN_HEIGHT || IsWall(bullets[i].x, bullets[i].y)) { //맵 끝 또는 장애물(벽)과의 충돌 처리
                bullets[i].active = false; //투사체 비활성화
                continue;
            }

            SDL_Rect bRect = { (int)bullets[i].x, (int)bullets[i].y, PROJECTILE_SIZE, PROJECTILE_SIZE };

            SDL_Texture* targetTexture = nullptr;
            if (bullets[i].owner == 0) { //플레이어
                targetTexture = gProjectileTexture;
            }
            else { //몹
                targetTexture = gEnemyProjectileTexture; //몹 전용 텍스처
            }

            if (targetTexture != nullptr) {
                //날아가는 방향 계산 (라디안 -> 도 변환)
                double angle = atan2(bullets[i].dirY, bullets[i].dirX) * (180.0 / M_PI);

                //이미지 출력
                SDL_RenderCopyEx(renderer, targetTexture, NULL, &bRect, angle, NULL, SDL_FLIP_NONE);
            }
            else {
                //이미지 로드 실패 시 백업용 사각형
                if (bullets[i].owner == 0) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); //노란색
                else SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); //빨간색

                SDL_RenderFillRect(renderer, &bRect);
            }
        }
    }
}
