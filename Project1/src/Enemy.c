#include "Enemy.h"
#include "MapSystem.hpp"
#include "Projectile.hpp"
#include "MapData.hpp"
#include <stdlib.h>
#include <math.h>

static Enemy worldEnemies[MAX_ROOMS_X][MAX_ROOMS_Y][MAX_ENEMIES_PER_ROOM];
static bool is_enemy_created[MAX_ROOMS_X][MAX_ROOMS_Y] = { false };
static Enemy currentEnemies[MAX_ENEMIES_PER_ROOM];
extern Projectile bullets[MAX_PROJECTILES];

void LoadEnemiesForRoom(int roomX, int roomY) {
    int roomType = roomNodes[roomX][roomY].roomType;

    if (
        roomType == ROOM_START ||
        roomType == ROOM_MAZE ||
        roomType == ROOM_QUIZ
        )
    {
        for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
            currentEnemies[i].active = false;
        }

        return;
    }

    if (!is_enemy_created[roomX][roomY]) {
        int count = (rand() % MAX_ENEMIES_PER_ROOM) + 1;
        for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
            if (i < count) {
                currentEnemies[i].active = true;
                bool found = false;

                while (!found) {
                    float x = (float)(rand() % (SCREEN_WIDTH - 100) + 50);

                    float y = (float)(rand() % (SCREEN_HEIGHT - 100) + 50);

                    if (CanMove(x, y)) {
                        currentEnemies[i].x = x;
                        currentEnemies[i].y = y;
                        found = true;
                    }
                }
                currentEnemies[i].spawnTime = SDL_GetTicks();
                currentEnemies[i].nextDodgeTime = SDL_GetTicks() + 5000; 
                currentEnemies[i].dodgeEndTime = 0;

                int r = rand() % 100;

                if (r < 65)
                    currentEnemies[i].type = ENEMY_NORMAL;

                else if (r <= 90)
                    currentEnemies[i].type = ENEMY_NINJA;

                else
                    currentEnemies[i].type = ENEMY_SUICIDE;

                currentEnemies[i].state = NINJA_IDLE;
                currentEnemies[i].stateStartTime = SDL_GetTicks();
                currentEnemies[i].warning = false;
                currentEnemies[i].speed = ENEMY_SPEED; // Constants.h에서 초당 픽셀(예: 150.0f)로 설정
                currentEnemies[i].lastAttackTime = SDL_GetTicks();
                currentEnemies[i].nextAttackDelay = ENEMY_ATTACK_MIN + (rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN));
                currentEnemies[i].moveDirX = 0.0f;
                currentEnemies[i].moveDirY = 0.0f;
                currentEnemies[i].nextMoveDecisionTime = SDL_GetTicks() + (rand() % 500);
                currentEnemies[i].explodeTime = SDL_GetTicks() + 5000;
                //몹마다 개별적인 이동 방향을 저장 (부들거림 방지)
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
    if (currentRoom->mapData[gridY][gridX] != 0 ||
        currentRoom->mapData[gridY][gridRight] != 0 ||
        currentRoom->mapData[gridBottom][gridX] != 0 ||
        currentRoom->mapData[gridBottom][gridRight] != 0)
    {
        return false;
    }
    return true;
}

//몹 출력 및 업데이트(이동,공격) 함수
void UpdateAndDrawEnemies(SDL_Renderer* renderer,
    float playerX,
    float playerY,
    SDL_Texture* enemyTexture,
    float deltaTime)
{
    Uint32 now = SDL_GetTicks();

    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        if (!currentEnemies[i].active)
            continue;

        Enemy* e = &currentEnemies[i];

        // 일반 몹
        if (e->type == ENEMY_NORMAL) {
            if (now >= e->nextMoveDecisionTime) {
                float dx = playerX - e->x;
                float dy = playerY - e->y;

                float len = sqrtf(dx * dx + dy * dy);

                if (len > 0.001f) {
                    dx /= len;
                    dy /= len;
                }

                float angle = atan2f(dy, dx);

                // ±30도 랜덤
                angle += ((rand() % 61) - 30) * 0.0174533f;

                if (len < 180.0f) {
                    e->moveDirX = -cosf(angle);
                    e->moveDirY = -sinf(angle);
                }
                else if (len > 300.0f) {
                    e->moveDirX = cosf(angle);
                    e->moveDirY = sinf(angle);
                }
                else {
                    e->moveDirX = 0.0f;
                    e->moveDirY = 0.0f;
                }

                e->nextMoveDecisionTime = now + 500 + rand() % 400;
            }

            float nextX = e->x + e->moveDirX * e->speed * deltaTime;

            float nextY = e->y + e->moveDirY * e->speed * deltaTime;

            if (CanMove(nextX, nextY)) {
                e->x = nextX;
                e->y = nextY;
            }

            // 공격
            if (now - e->lastAttackTime > e->nextAttackDelay) {
                FireEnemyProjectile( e->x, e->y, playerX + PLAYER_SIZE * 0.5f, playerY + PLAYER_SIZE * 0.5f);

                e->lastAttackTime = now;

                e->nextAttackDelay = ENEMY_ATTACK_MIN + rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN);
            }
        }

        // 자폭 몹
        else if (e->type == ENEMY_SUICIDE) {
            float dx = playerX - e->x;
            float dy = playerY - e->y;

            float len = sqrtf(dx * dx + dy * dy);

            if (len > 0.001f) {
                dx /= len;
                dy /= len;
            }

            float nextX = e->x + dx * SUICIDE_SPEED * deltaTime;

            float nextY = e->y + dy * SUICIDE_SPEED * deltaTime;

            if (CanMove(nextX, nextY)) {
                e->x = nextX;
                e->y = nextY;
            }

            // 자폭
            if (now >= e->explodeTime) {
                float pdx = playerX - e->x;
                float pdy = playerY - e->y;

                float dist = sqrtf(pdx * pdx + pdy * pdy);

                if (dist < 120.0f) {
                    // 플레이어 데미지 부분
                }

                e->active = false;
                continue;
            }
        }

        // 닌자 몹
        else if (e->type == ENEMY_NINJA) {
            float dx = playerX - e->x;
            float dy = playerY - e->y;

            float len = sqrtf(dx * dx + dy * dy);

            if (len > 0.001f) {
                dx /= len;
                dy /= len;
            }

            float nextX = e->x;
            float nextY = e->y;

            // 목표거리 유지
            if (len > NINJA_KEEP_DISTANCE) {
                nextX += dx * e->speed * deltaTime;
                nextY += dy * e->speed * deltaTime;
            }
            else if (len < NINJA_KEEP_DISTANCE * 0.8f) {
                nextX -= dx * e->speed * deltaTime;
                nextY -= dy * e->speed * deltaTime;
            }

            if (CanMove(nextX, nextY)) {
                e->x = nextX;
                e->y = nextY;
            }

            // 회피모드
            for (int b = 0; b < MAX_PROJECTILES; b++) {
                if (!bullets[b].active)
                    continue;

                if (bullets[b].owner != 0)
                    continue;

                float bx = bullets[b].x;
                float by = bullets[b].y;

                float vx = bullets[b].dirX;
                float vy = bullets[b].dirY;

                float ex = e->x + ENEMY_SIZE * 0.5f;
                float ey = e->y + ENEMY_SIZE * 0.5f;

                float toEnemyX = ex - bx;
                float toEnemyY = ey - by;

                float projection = toEnemyX * vx + toEnemyY * vy;

                if (projection < 0.0f)
                    continue;

                if (projection > 250.0f)
                    continue;

                float closestX = bx + vx * projection;

                float closestY = by + vy * projection;

                float missX = ex - closestX;

                float missY = ey - closestY;

                float missDistance = sqrtf( missX * missX + missY * missY);

                if (missDistance > 20.0f)
                    continue;

                if (now - e->lastEmergencyDodgeTime < 3000)
                    continue;

                e->lastEmergencyDodgeTime = now;

                e->isDodging = true;
                e->isInvincible = true;

                e->dodgeEndTime =
                    now + 1500;

                float dodgeX = -vy;
                float dodgeY = vx;

                if (rand() % 2) {
                    dodgeX *= -1.0f;
                    dodgeY *= -1.0f;
                }

                float dodgeDistance = 180.0f;

                float newX = e->x + dodgeX * dodgeDistance;

                float newY = e->y + dodgeY * dodgeDistance;

                if (CanMove(newX, newY)) {
                    e->x = newX;
                    e->y = newY;
                }

                break;
            }

            if (e->isDodging) {
                if (now >= e->dodgeEndTime) {
                    e->isDodging = false;
                    e->isInvincible = false;
                }
            }

            // 돌진 공격

            if (now - e->lastAttackTime >= 3000) {
                float dashDx = playerX - e->x;
                float dashDy = playerY - e->y;

                float dashLen = sqrtf(dashDx * dashDx + dashDy * dashDy);

                if (dashLen > 0.001f) {
                    dashDx /= dashLen;
                    dashDy /= dashLen;

                    float dashX = e->x + dashDx * NINJA_DASH_DISTANCE * 1.5f;

                    float dashY = e->y + dashDy * NINJA_DASH_DISTANCE * 1.5f;

                    float dashDist = NINJA_DASH_DISTANCE * 1.5f;

                    while (dashDist > 10)
                    {
                        float dashX = e->x + dashDx * dashDist;

                        float dashY = e->y + dashDy * dashDist;

                        if (CanMove(dashX, dashY)) {
                            e->x = dashX;
                            e->y = dashY;
                            break;
                        }

                        dashDist -= 20;
                    }
                }

                e->lastAttackTime = now;
            }
        }

        // 출력
        SDL_Rect enemyRect = { (int)e->x, (int)e->y, ENEMY_SIZE, ENEMY_SIZE };

        if (enemyTexture) {
            if (e->type == ENEMY_NORMAL) {
                SDL_SetTextureColorMod( enemyTexture, 255, 255, 255); // 원본색
            }
            else if (e->type == ENEMY_SUICIDE) {
                SDL_SetTextureColorMod( enemyTexture, 255, 220, 80); // 노랑
            }
            else if (e->type == ENEMY_NINJA) {
                SDL_SetTextureColorMod( enemyTexture, 100, 255, 100); // 초록
            }

            SDL_RenderCopy( renderer, enemyTexture, NULL, &enemyRect);

            SDL_SetTextureColorMod( enemyTexture, 255, 255, 255); // 원상복구
        }
        else {
            if (e->type == ENEMY_NORMAL)
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            else if (e->type == ENEMY_SUICIDE)
            {
                Uint32 remain = e->explodeTime - SDL_GetTicks();

                if (remain < 2000) {
                    int interval = remain > 1000 ? 150 : 70;

                    bool blink = (SDL_GetTicks() / interval) % 2;

                    if (blink)
                        SDL_SetTextureColorMod(enemyTexture, 255, 255, 0);
                    else
                        SDL_SetTextureColorMod(enemyTexture, 255, 100, 0);
                }
                else {
                    SDL_SetTextureColorMod(
                        enemyTexture,
                        255, 255, 255);
                }
            }

            else
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

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

                if (currentEnemies[e].isInvincible){
                    bullets[b].active = false;
                    continue;
                }

                if (currentEnemies[e].type == ENEMY_NINJA && currentEnemies[e].isInvincible) {
                    bullets[b].active = false;
                    continue;
                }

                currentEnemies[e].active = false;
                bullets[b].active = false;
            }
        }
    }
}

bool AreEnemiesAlive(void) {
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        if (currentEnemies[i].active)
            return true;
    }

    return false;
}

bool CanSpawnEnemy(float x, float y)
{
    int left = (int)x / TILE_SIZE;

    int top = (int)y / TILE_SIZE;

    int right = (int)(x + ENEMY_SIZE) / TILE_SIZE;

    int bottom = (int)(y + ENEMY_SIZE) / TILE_SIZE;

    if (currentRoom->mapData[top][left] != 0)
        return false;

    if (currentRoom->mapData[top][right] != 0)
        return false;

    if (currentRoom->mapData[bottom][left] != 0)
        return false;

    if (currentRoom->mapData[bottom][right] != 0)
        return false;

    return true;
}
