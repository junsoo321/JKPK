#include "Enemy.h"
#include "MapSystem.h"
#include "Projectile.hpp"
#include "MapData.h"
#include <stdlib.h>
#include <math.h>

static Enemy worldEnemies[MAX_ROOMS_X][MAX_ROOMS_Y][MAX_ENEMIES_PER_ROOM];
static bool is_enemy_created[MAX_ROOMS_X][MAX_ROOMS_Y] = { false };
static Enemy currentEnemies[MAX_ENEMIES_PER_ROOM];

void LoadEnemiesForRoom(int roomX, int roomY) {
    if (!is_enemy_created[roomX][roomY]) {
        int count = (rand() % MAX_ENEMIES_PER_ROOM) + 1;
        for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
            if (i < count) {
                currentEnemies[i].active = true;
                currentEnemies[i].x = (float)(rand() % (SCREEN_WIDTH - 100) + 50);
                currentEnemies[i].y = (float)(rand() % (SCREEN_HEIGHT - 100) + 50);
                currentEnemies[i].speed = ENEMY_SPEED; // Constants.h에서 초당 픽셀(예: 150.0f)로 설정
                currentEnemies[i].lastAttackTime = SDL_GetTicks();
                currentEnemies[i].nextAttackDelay = ENEMY_ATTACK_MIN + (rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN));

                // [추가] 몹마다 개별적인 이동 방향을 저장 (부들거림 방지)
                currentEnemies[i].dirX = (float)((rand() % 3) - 1);
                currentEnemies[i].dirY = (float)((rand() % 3) - 1);
            }
            else {
                currentEnemies[i].active = false;
            }
        }
        is_enemy_created[roomX][roomY] = true;
    }
    else {
        for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
            currentEnemies[i] = worldEnemies[roomX][roomY][i];
        }
    }
}

void SaveEnemiesForRoom(int roomX, int roomY) {
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        worldEnemies[roomX][roomY][i] = currentEnemies[i];
    }
}

bool CanMove(float nextX, float nextY) {
    if (nextX < 5 || nextX + ENEMY_SIZE > SCREEN_WIDTH) return false;
    if (nextY < 5 || nextY + ENEMY_SIZE > SCREEN_HEIGHT) return false;

    int gridX = (int)nextX / TILE_SIZE;
    int gridY = (int)nextY / TILE_SIZE;
    int gridRight = (int)(nextX + ENEMY_SIZE) / TILE_SIZE;
    int gridBottom = (int)(nextY + ENEMY_SIZE) / TILE_SIZE;

    // 몹 히트박스의 네 모서리 중 하나라도 벽에 걸리면 이동 불가
    if (worldMap[gridY][gridX] != 0 || worldMap[gridY][gridRight] != 0 || worldMap[gridBottom][gridX] != 0 || worldMap[gridBottom][gridRight] != 0) {
        return false;
    }
    return true;
}

//몹 출력 및 업데이트(이동,공격) 함수
void UpdateAndDrawEnemies(SDL_Renderer* renderer, float playerX, float playerY, SDL_Texture* enemyTexture, float deltaTime) {
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        if (!currentEnemies[i].active) continue;

        //이동 로직
        if (rand() % 100 < 10) { // 10% 확률로 새로운 방향으로 이동 시도
            float moveX = (float)((rand() % 3) - 1) * ENEMY_SPEED * deltaTime;
            float moveY = (float)((rand() % 3) - 1) * ENEMY_SPEED * deltaTime;

            //이동할 좌표 생성
            float nextX = currentEnemies[i].x + moveX;
            float nextY = currentEnemies[i].y + moveY;

            // CanMove가 true일 때만 현재 좌표를 업데이트
            if (CanMove(nextX, nextY)) {
                currentEnemies[i].x = nextX;
                currentEnemies[i].y = nextY;
            }
        }

        float nextX = currentEnemies[i].x + (currentEnemies[i].dirX * currentEnemies[i].speed * deltaTime);
        float nextY = currentEnemies[i].y + (currentEnemies[i].dirY * currentEnemies[i].speed * deltaTime);

        if (CanMove(nextX, nextY)) {
            currentEnemies[i].x = nextX;
            currentEnemies[i].y = nextY;
        }
        else {
            // 벽에 부딪히면 반대 방향이나 정지 상태로 전환 (선택 사항)
            currentEnemies[i].dirX *= -1;
            currentEnemies[i].dirY *= -1;
        }

        //공격 로직
        Uint32 now = SDL_GetTicks();
        if (now - currentEnemies[i].lastAttackTime > currentEnemies[i].nextAttackDelay) {
            FireEnemyProjectile(currentEnemies[i].x, currentEnemies[i].y,
                playerX + (PLAYER_SIZE / 2.0f), playerY + (PLAYER_SIZE / 2.0f));
            currentEnemies[i].lastAttackTime = now;
            currentEnemies[i].nextAttackDelay = ENEMY_ATTACK_MIN + (rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN));
        }

        // 3. 출력 로직
        SDL_Rect enemyRect = { (int)currentEnemies[i].x, (int)currentEnemies[i].y, ENEMY_SIZE, ENEMY_SIZE }; //객체의 원 좌표(왼쪽아래) 기준 크기만큼 구역 설정
        if (enemyTexture) {
            SDL_RenderCopy(renderer, enemyTexture, NULL, &enemyRect);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &enemyRect);
        }
    }
}

void CheckEnemyCollision(void* bulletArray) {
    Projectile* bullets = (Projectile*)bulletArray;
    for (int b = 0; b < MAX_PROJECTILES; b++) {
        if (!bullets[b].active || bullets[b].owner == 1) continue;

        for (int e = 0; e < MAX_ENEMIES_PER_ROOM; e++) {
            if (!currentEnemies[e].active) continue;

            // AABB 충돌 체크
            if (bullets[b].x < currentEnemies[e].x + ENEMY_SIZE &&
                bullets[b].x + PROJECTILE_SIZE > currentEnemies[e].x &&
                bullets[b].y < currentEnemies[e].y + ENEMY_SIZE &&
                bullets[b].y + PROJECTILE_SIZE > currentEnemies[e].y) {

                currentEnemies[e].active = false;
                bullets[b].active = false;
            }
        }
    }
}
