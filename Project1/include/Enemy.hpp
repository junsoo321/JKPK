#pragma once

#include <SDL.h>
#include <stdbool.h>
#include "Constants.h"

typedef enum
{
    NINJA_IDLE,
    NINJA_DASH_PREPARE,
    NINJA_DASH,
    NINJA_RECOVER
} NinjaState;

typedef enum
{
    ENEMY_NORMAL,
    ENEMY_SUICIDE,
    ENEMY_NINJA
} EnemyType;

typedef struct
{
    float x, y;

    float speed;
    float dirX;
    float dirY;
    float moveDirX;
    float moveDirY;
    Uint32 nextMoveDecisionTime;
    bool active;

    EnemyType type;

    Uint32 spawnTime;

    //투사체(일반몹)
    Uint32 lastAttackTime;
    Uint32 nextAttackDelay;

    //자폭몹
    Uint32 explodeTime;

    //닌자몹
    Uint32 skillStartTime;
    Uint32 skillCooldownEnd;
    Uint32 dodgeEndTime;
    Uint32 nextDodgeTime;
    bool isDodging;
    bool isInvincible;
    Uint32 lastEmergencyDodgeTime;
    Uint32 stateStartTime;

    int state;

    bool warning;
} Enemy;


#ifdef __cplusplus //C++에서 이 헤더를 포함해도 C 언어 함수 이름을 그대로 사용할 수 있도록 설정
extern "C" {
#endif
    //맵 진입 시 몹 속성 초기값 설정 및 불러오는 함수
    void LoadEnemiesForRoom(int roomX, int roomY);

    //맵 이동 시 현재 존재하는 몹들의 정보(위치, 개수, 마지막 공격시간, 다음 공격시간)를 저장하는 함수
    void SaveEnemiesForRoom(int roomX, int roomY);

    //몹 벽 충돌 체크 함수
    bool CanMove(float nextX, float nextY);

    //몹 출력 및 업데이트(이동,공격) 함수
    void UpdateAndDrawEnemies(SDL_Renderer* renderer, float playerX, float playerY, SDL_Texture* enemyTexture, float deltaTime, struct PlayerData* player);

    //투사체 충돌 체크 함수
    void CheckEnemyCollision(void* projectileArray);

    bool AreEnemiesAlive(void);

#ifdef __cplusplus //C++에서 이 헤더를 포함해도 C 언어 함수 이름을 그대로 사용할 수 있도록 설정
}
#endif
