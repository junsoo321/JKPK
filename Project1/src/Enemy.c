#include "Enemy.h"
#include "MapSystem.h"
#include "Projectile.hpp"
#include "MapData.h"
#include <stdlib.h>
#include <math.h>

//이미 방문한 맵에 다시 방문했을때 몹을 새로 생성하지 않고, 기존에 있던 몹을 그대로 다시 복원하는 용도
static Enemy worldEnemies[MAX_ROOMS_X][MAX_ROOMS_Y][MAX_ENEMIES_PER_ROOM];  //월드 전체의 몹 데이터를 저장 (방X, 방Y, 몹 인덱스)
static bool is_enemy_created[MAX_ROOMS_X][MAX_ROOMS_Y] = { false };         //맵 별로 몹이 이미 생성되었는지 여부
static Enemy currentEnemies[MAX_ENEMIES_PER_ROOM];      //맵의 몹 데이터를 저장(몹 인덱스)

//맵 진입 시 몹 속성 초기값 설정 및 불러오는 함수
void LoadEnemiesForRoom(int roomX, int roomY) {
    // 처음 방문일때만 랜덤 생성 (최소 1마리 이상)Initialized
    if (!is_enemy_created[roomX][roomY]) {
        int count = (rand() % MAX_ENEMIES_PER_ROOM) + 1;
        for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) { //각 몹의 인덱스
            if (i < count) { //0~랜덤 몹 수 사이의 몹만 사용
                currentEnemies[i].active = true;
                currentEnemies[i].x = (float)(rand() % (SCREEN_WIDTH - 100) + 50);  //랜덤 좌표(테두리 제외)
                currentEnemies[i].y = (float)(rand() % (SCREEN_HEIGHT - 100) + 50);
                currentEnemies[i].speed = ENEMY_SPEED;
                currentEnemies[i].lastAttackTime = SDL_GetTicks(); //마지막 공격 시간을 생성시간으로 간주
                currentEnemies[i].nextAttackDelay = ENEMY_ATTACK_MIN + (rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN)); //다음 공격 시간(딜레이) 적용

            }
            else {
                currentEnemies[i].active = false; //랜덤 개수 이상의 몹은 비활성화
            }
        }
        is_enemy_created[roomX][roomY] = true; //현재 맵의 몹은 생성됨(true)
    }
    else {
        // 이미 방문했다면 저장된 위치 로드
        for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
            currentEnemies[i] = worldEnemies[roomX][roomY][i];
        }
    }
}

//맵 이동 시 현재 존재하는 몹들의 정보(위치, 개수, 마지막 공격시간, 다음 공격시간)를 저장하는 함수
void SaveEnemiesForRoom(int roomX, int roomY) {
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        worldEnemies[roomX][roomY][i] = currentEnemies[i]; //현재 맵 몹 정보를 월드 몹 정보에 저장
    }
}

//몹들이 맵 밖으로 나가거나 벽을 통과하는것을 방지하는 함수
bool CanMove(float nextX, float nextY) {
    // 1. 현재 맵 밖으로 나가는 것 방지
    if (nextX < 5 || nextX + ENEMY_SIZE > SCREEN_WIDTH) return false;
    if (nextY < 5 || nextY + ENEMY_SIZE > SCREEN_HEIGHT) return false;

    // 2. 구역 기반 벽 체크
    // 픽셀 좌표를 구역 크기로 나누어 해당 칸이 벽(1)인지 확인
    int gridX = (int)nextX / TILE_SIZE;
    int gridY = (int)nextY / TILE_SIZE;
    int gridRight = (int)(nextX + ENEMY_SIZE) / TILE_SIZE;
    int gridBottom = (int)(nextY + ENEMY_SIZE) / TILE_SIZE;

    // 몹 히트박스의 네 모서리 중 하나라도 벽에 걸리면 이동 불가
    if (worldMap[gridY][gridX] == 1 || worldMap[gridY][gridRight] == 1 || worldMap[gridBottom][gridX] == 1 || worldMap[gridBottom][gridRight] == 1) {
        return false;
    }

    return true;
}

//몹 출력 및 업데이트(이동,공격) 함수
void UpdateAndDrawEnemies(SDL_Renderer* renderer, float playerX, float playerY) {
    for (int i = 0; i < MAX_ENEMIES_PER_ROOM; i++) {
        if (!currentEnemies[i].active) continue;    //몹이 비활성 상태일 경우

        //이동 로직
        //이동속도로에 의해서만 이동할 경우, 이동 시간이 되자마자 움직이므로, 너무 단조로워짐
        if (rand() % 100 < 10) { // 10% 확률로 새로운 방향으로 이동 시도
            float moveX = (float)((rand() % 3) - 1) * ENEMY_SPEED;  //(rand() % 3) - 1)의 값은 -1,0,1중 하나가 랜덤하게 나옴, 즉, 왼쪽으로이동, 유지, 오른쪽으로 이동 중 하나가 랜덤하게 나옴
            float moveY = (float)((rand() % 3) - 1) * ENEMY_SPEED;  //위로이동, 유지, 아래로 이동 중 하나가 랜덤하게 나옴

            //이동할 좌표 생성
            float nextX = currentEnemies[i].x + moveX;
            float nextY = currentEnemies[i].y + moveY;

            // CanMove가 true일 때만 현재 좌표를 업데이트
            if (CanMove(nextX, nextY)) {
                currentEnemies[i].x = nextX;
                currentEnemies[i].y = nextY;
            }
        }

        //공격 로직
        Uint32 now = SDL_GetTicks(); //현재 시각
        if (now - currentEnemies[i].lastAttackTime > currentEnemies[i].nextAttackDelay) { //현재 시각과 마지막 공격 시각의 차이가 딜레이 보다 클 경우(딜레이 시간만큼 지났을 경우)
            FireEnemyProjectile(currentEnemies[i].x, currentEnemies[i].y, playerX + (PLAYER_SIZE / 2), playerY + (PLAYER_SIZE / 2)); //플레이어 현재 좌표 + 플레이어 크기의 절반 = 플레이어의 정중앙 좌표
            currentEnemies[i].lastAttackTime = now; //마지막 공격 시각 갱신
            currentEnemies[i].nextAttackDelay = ENEMY_ATTACK_MIN + (rand() % (ENEMY_ATTACK_MAX - ENEMY_ATTACK_MIN));  //다음 딜레이 랜덤 생성
        }

        //출력 로직
        SDL_Rect enemyRect = { (int)currentEnemies[i].x, (int)currentEnemies[i].y, ENEMY_SIZE, ENEMY_SIZE }; //객체의 원 좌표(왼쪽아래) 기준 크기만큼 구역 설정
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 기본 빨간색
        SDL_RenderFillRect(renderer, &enemyRect); //객체(몹) 생성
    }
}

//투사체 충돌 체크 함수
void CheckEnemyCollision(void* bulletArray) {
    Projectile* bullets = (Projectile*)bulletArray;
    for (int b = 0; b < MAX_PROJECTILES; b++) {
        if (!bullets[b].active) continue; //투사체가 비활성화 상태일 경우

        if (bullets[b].owner == 1) continue; //몹이 생성한 투사체일 경우 충돌 체크 x

        for (int e = 0; e < MAX_ENEMIES_PER_ROOM; e++) {
            if (!currentEnemies[e].active) continue;

            // 단순 사각형 충돌 (AABB 알고리즘)
            if (bullets[b].x < currentEnemies[e].x + ENEMY_SIZE && bullets[b].x + PROJECTILE_SIZE > currentEnemies[e].x && bullets[b].y < currentEnemies[e].y + ENEMY_SIZE && bullets[b].y + PROJECTILE_SIZE > currentEnemies[e].y) { //히트박스와 투사체가 충돌했을 경우
                currentEnemies[e].active = false; // 몹 제거
                bullets[b].active = false;        // 투사체 제거
            }
        }
    }
}

