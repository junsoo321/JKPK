#include "Enemy.hpp"
#include "Player.hpp"
#include "MapSystem.hpp"
#include "Projectile.hpp"
#include "GameState.hpp"
#include "MapData.hpp"
#include "ImageManager.hpp"
#include <stdlib.h>
#include <math.h>

static Enemy worldEnemies[MAX_ROOMS_X][MAX_ROOMS_Y][MAX_ENEMIES_PER_ROOM];
static bool is_enemy_created[MAX_ROOMS_X][MAX_ROOMS_Y] = { false };
static Enemy currentEnemies[MAX_ENEMIES_PER_ROOM];
extern Projectile bullets[MAX_PROJECTILES];

//특정 방 진입 시 적 목록 초기화 또는 기존 데이터 로드
void LoadEnemiesForRoom(int roomX, int roomY)
{
    int roomType = roomNodes[roomX][roomY].roomType;

    // 안전 구역 및 미로/퀴즈 방은 스폰 제외
    if (roomType == ROOM_START || roomType == ROOM_MAZE || roomType == ROOM_QUIZ) {
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

                // 확률별 몬스터 타입 가중치 설정 (일반 65%, 닌자 25%, 자폭 10%)
                int r = rand() % 100;
                if (r < 65)       currentEnemies[i].type = ENEMY_NORMAL;
                else if (r <= 90) currentEnemies[i].type = ENEMY_NINJA;
                else              currentEnemies[i].type = ENEMY_SUICIDE;

                currentEnemies[i].state = NINJA_IDLE;
                currentEnemies[i].stateStartTime = SDL_GetTicks();
                currentEnemies[i].warning = false;
                if (currentEnemies[i].type == ENEMY_NINJA)
                    currentEnemies[i].skillCooldownEnd = SDL_GetTicks() + 2000 + (rand() % 1000);
                currentEnemies[i].speed = ENEMY_SPEED;
                currentEnemies[i].lastAttackTime = SDL_GetTicks();
                currentEnemies[i].nextAttackDelay = ENEMY_ATTACK_MIN + (rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN));
                currentEnemies[i].moveDirX = 0.0f;
                currentEnemies[i].moveDirY = 0.0f;
                currentEnemies[i].nextMoveDecisionTime = SDL_GetTicks() + (rand() % 500);
                currentEnemies[i].explodeTime = SDL_GetTicks() + 5000;
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

//방을 이탈할 때 현재 몹들의 상태(체력, 위치 등)를 전역 배열에 백업
void SaveEnemiesForRoom(int roomX, int roomY)
{
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        worldEnemies[roomX][roomY][i] = currentEnemies[i];
    }
}

//몹 히트박스가 차지하는 모든 타일을 스윕해 벽 충돌 판정
bool CanMove(float nextX, float nextY)
{
    if (nextX < 5 || nextX + ENEMY_SIZE > SCREEN_WIDTH) return false;
    if (nextY < 5 || nextY + ENEMY_SIZE > SCREEN_HEIGHT) return false;

    int colMin = (int)nextX / TILE_SIZE;
    int colMax = (int)(nextX + ENEMY_SIZE - 1) / TILE_SIZE;
    int rowMin = (int)nextY / TILE_SIZE;
    int rowMax = (int)(nextY + ENEMY_SIZE - 1) / TILE_SIZE;

    for (int row = rowMin; row <= rowMax; row++) {
        for (int col = colMin; col <= colMax; col++) {
            if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return false;
            if (currentRoom->mapData[row][col] != 0) return false;
        }
    }
    return true;
}

//타입별 AI 동작(거리 유지, 투사체 예측 횡회피, 돌진) 연산 및 프레임 렌더링
void UpdateAndDrawEnemies(SDL_Renderer* renderer, float playerX, float playerY, float deltaTime, PlayerData* player)
{
    Uint32 now = SDL_GetTicks();
    bool paused = (gGameState == GAME_PAUSE);

    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        if (!currentEnemies[i].active) continue;

        Enemy* e = &currentEnemies[i];

        // 1. 일반 타입 AI (거리별 도망 및 추적)
        if (e->type == ENEMY_NORMAL && !paused) {
            if (now >= e->nextMoveDecisionTime) {
                float dx = playerX - e->x;
                float dy = playerY - e->y;
                float len = sqrtf(dx * dx + dy * dy);

                if (len > 0.001f) { dx /= len; dy /= len; }

                float angle = atan2f(dy, dx);
                angle += ((rand() % 61) - 30) * 0.0174533f; // ±30도 랜덤 편차

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
            if (CanMove(nextX, nextY)) { e->x = nextX; e->y = nextY; }

            if (now - e->lastAttackTime > e->nextAttackDelay) {
                FireEnemyProjectile(e->x, e->y, playerX + PLAYER_SIZE * 0.5f, playerY + PLAYER_SIZE * 0.5f);
                e->lastAttackTime = now;
                e->nextAttackDelay = ENEMY_ATTACK_MIN + rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN);
            }
        }
        // 2. 자폭 타입 AI (무조건 돌진 → 근접 시 폭발)
        else if (e->type == ENEMY_SUICIDE && !paused) {
            float dx = playerX - e->x;
            float dy = playerY - e->y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist > 0.001f) { dx /= dist; dy /= dist; }

            float nextX = e->x + dx * SUICIDE_SPEED * deltaTime;
            float nextY = e->y + dy * SUICIDE_SPEED * deltaTime;
            if (CanMove(nextX, nextY)) { e->x = nextX; e->y = nextY; }

            // 폭발 판정: 근접 거리 이내 또는 타이머 만료
            bool explode = (dist < SUICIDE_EXPLOSION_RANGE) || (now >= e->explodeTime);
            if (explode) {
                if (dist < SUICIDE_EXPLOSION_RANGE && player && !player->isInvincible) {
                    player->hp -= SUICIDE_EXPLOSION_DAMAGE;
                    player->hurtTimer = 0.3f;
                    player->isInvincible = true;
                    player->invincibleEndTime = now + 2000;
                }
                e->active = false;
                continue;
            }
        }
        // 3. 닌자 타입 AI (거리 유지 + 플레이어 총알 벡터 예측 횡회피 + 3초 주기 칼날 돌진)
        else if (e->type == ENEMY_NINJA && !paused) {
            float dx = playerX - e->x;
            float dy = playerY - e->y;
            float len = sqrtf(dx * dx + dy * dy);

            if (len > 0.001f) { dx /= len; dy /= len; }

            float nextX = e->x;
            float nextY = e->y;

            if (len > NINJA_KEEP_DISTANCE) {
                nextX += dx * e->speed * deltaTime;
                nextY += dy * e->speed * deltaTime;
            }
            else if (len < NINJA_KEEP_DISTANCE * 0.8f) {
                nextX -= dx * e->speed * deltaTime;
                nextY -= dy * e->speed * deltaTime;
            }
            if (CanMove(nextX, nextY)) { e->x = nextX; e->y = nextY; }

            // 플레이어 탄환 실시간 벡터 투영 회피 연산
            for (int b = 0; b < MAX_PROJECTILES; b++) {
                if (!bullets[b].active || bullets[b].owner != 0) continue;

                float bx = bullets[b].x; float by = bullets[b].y;
                float vx = bullets[b].dirX; float vy = bullets[b].dirY;
                float ex = e->x + ENEMY_SIZE * 0.5f; float ey = e->y + ENEMY_SIZE * 0.5f;

                float toEnemyX = ex - bx;
                float toEnemyY = ey - by;
                float projection = toEnemyX * vx + toEnemyY * vy;

                if (projection < 0.0f || projection > 250.0f) continue;

                float closestX = bx + vx * projection;
                float closestY = by + vy * projection;
                float missX = ex - closestX;
                float missY = ey - closestY;

                if (sqrtf(missX * missX + missY * missY) > 20.0f) continue;
                if (now - e->lastEmergencyDodgeTime < 3000) continue;

                // 조건 충족 시 수직 벡터 방향으로 긴급 구르기 수행
                e->lastEmergencyDodgeTime = now;
                e->isDodging = true;
                e->isInvincible = true;
                e->dodgeEndTime = now + 1500;

                float dodgeX = -vy; float dodgeY = vx;
                if (rand() % 2) { dodgeX *= -1.0f; dodgeY *= -1.0f; }

                float dodgeDistance = 180.0f;
                float newX = e->x + dodgeX * dodgeDistance;
                float newY = e->y + dodgeY * dodgeDistance;

                if (CanMove(newX, newY)) { e->x = newX; e->y = newY; }
                break;
            }

            if (e->isDodging && now >= e->dodgeEndTime) {
                e->isDodging = false;
                e->isInvincible = false;
            }

            // 레이캐스트 스타일 단기 선형 후퇴 검증 방식의 칼날 돌진 공격
            if (now - e->lastAttackTime >= 3000) {
                float dashDx = playerX - e->x;
                float dashDy = playerY - e->y;
                float dashLen = sqrtf(dashDx * dashDx + dashDy * dashDy);

                if (dashLen > 0.001f) {
                    dashDx /= dashLen; dashDy /= dashLen;
                    float dashDist = NINJA_DASH_DISTANCE * 1.5f;

                    while (dashDist > 10) {
                        float dashX = e->x + dashDx * dashDist;
                        float dashY = e->y + dashDy * dashDist;

                        if (CanMove(dashX, dashY)) {
                            e->x = dashX; e->y = dashY;
                            break;
                        }
                        dashDist -= 20; // 벽에 막힐 경우 도달할 수 있는 최대 거리까지 양보 연산
                    }
                }
                e->lastAttackTime = now;
            }

            // 3방향 확산탄: 2.5초마다 플레이어 방향 ±15도로 발사
            if (now >= e->skillCooldownEnd) {
                float cx = playerX + PLAYER_SIZE * 0.5f;
                float cy = playerY + PLAYER_SIZE * 0.5f;
                float bx = e->x + ENEMY_SIZE * 0.5f;
                float by = e->y + ENEMY_SIZE * 0.5f;
                float dx = cx - bx, dy = cy - by;
                float d  = sqrtf(dx * dx + dy * dy);
                if (d > 0.001f) { dx /= d; dy /= d; }

                for (int s = -1; s <= 1; s++) {
                    float a  = atan2f(dy, dx) + s * 0.2618f; // 15도 간격
                    float tx = bx + cosf(a) * 100.0f;
                    float ty = by + sinf(a) * 100.0f;
                    FireEnemyProjectileEx(e->x, e->y, tx, ty, 3);
                }
                e->skillCooldownEnd = now + 2500;
            }
        }

        // 렌더 출력부
        SDL_Rect enemyRect = { (int)e->x, (int)e->y, ENEMY_SIZE, ENEMY_SIZE };

        SDL_Texture* drawTex = nullptr;
        if      (e->type == ENEMY_NORMAL)  drawTex = gNormalEnemyTex;
        else if (e->type == ENEMY_NINJA)   drawTex = gNinjaEnemyTex;
        else if (e->type == ENEMY_SUICIDE) drawTex = gSuicideEnemyTex;
        if (!drawTex) drawTex = nullptr;

        if (drawTex) {
            // 자폭 몬스터: 근접 시 붉은 점멸 경고
            if (e->type == ENEMY_SUICIDE) {
                float pdx = playerX - e->x;
                float pdy = playerY - e->y;
                float dist = sqrtf(pdx * pdx + pdy * pdy);
                if (dist < SUICIDE_WARN_RANGE) {
                    if ((now / 100) % 2 == 0) SDL_SetTextureColorMod(drawTex, 255, 80,  80);
                    else                       SDL_SetTextureColorMod(drawTex, 180, 40,  40);
                }
            }
            SDL_RenderCopy(renderer, drawTex, NULL, &enemyRect);
            SDL_SetTextureColorMod(drawTex, 255, 255, 255);
        }
        else {
            if      (e->type == ENEMY_NORMAL)  SDL_SetRenderDrawColor(renderer, 255,   0,   0, 255);
            else if (e->type == ENEMY_SUICIDE) SDL_SetRenderDrawColor(renderer, 255, 200,   0, 255);
            else                               SDL_SetRenderDrawColor(renderer,   0, 255, 255, 255);
            SDL_RenderFillRect(renderer, &enemyRect);
        }
    }
}

//총알 배열을 순회하며 피격 판정 및 무적(팅김) 처리
void CheckEnemyCollision(void* bulletArray)
{
    Projectile* localBullets = (Projectile*)bulletArray;
    for (int b = 0; b < MAX_PROJECTILES; b++) {
        if (!localBullets[b].active || localBullets[b].owner == 1) continue;

        for (int e = 0; e < MAX_ENEMIES_PER_ROOM; e++) {
            if (!currentEnemies[e].active) continue;

            float hitX = currentEnemies[e].x + ENEMY_HIT_OFFSET_X;
            float hitY = currentEnemies[e].y + ENEMY_HIT_OFFSET_Y;

            if (localBullets[b].x < hitX + ENEMY_HIT_W &&
                localBullets[b].x + PROJECTILE_SIZE > hitX &&
                localBullets[b].y < hitY + ENEMY_HIT_H &&
                localBullets[b].y + PROJECTILE_SIZE > hitY) {

                // 대쉬 혹은 무적 판정 상태이면 탄환만 지우고 스킵
                if (currentEnemies[e].isInvincible) {
                    localBullets[b].active = false;
                    continue;
                }
                currentEnemies[e].active = false;
                localBullets[b].active = false;
            }
        }
    }
}

bool AreEnemiesAlive(void)
{
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        if (currentEnemies[i].active) return true;
    }
    return false;
}

bool CanSpawnEnemy(float x, float y)
{
    return CanMove(x, y);
}

void SpawnEnemyAt(float x, float y, EnemyType type)
{
    Uint32 now = SDL_GetTicks();
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        if (!currentEnemies[i].active) {
            Enemy* e = &currentEnemies[i];
            *e = {};
            e->active             = true;
            e->x                  = x;
            e->y                  = y;
            e->type               = type;
            e->spawnTime          = now;
            e->speed              = ENEMY_SPEED;
            e->state              = NINJA_IDLE;
            e->stateStartTime     = now;
            e->lastAttackTime     = now;
            e->nextAttackDelay    = ENEMY_ATTACK_MIN + (rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN));
            e->nextMoveDecisionTime = now + (rand() % 500);
            e->explodeTime        = now + 5000;
            e->skillCooldownEnd   = now + 2000 + (rand() % 1000);
            e->nextDodgeTime      = now + 5000;
            e->dirX               = (float)((rand() % 3) - 1);
            e->dirY               = (float)((rand() % 3) - 1);
            break;
        }
    }
}
