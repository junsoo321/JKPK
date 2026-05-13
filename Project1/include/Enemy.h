#pragma once

#include <SDL.h>
#include <stdbool.h>
#include "Constants.h"

//몹 구조체 정의
typedef struct {
    float x, y;
    float speed;
    bool active;
    //공격 시점은 개별 측정이 아닌 SDL_GetTicks() 기준으로 결정
    Uint32 lastAttackTime; // 마지막으로 공격한 시각
    Uint32 nextAttackDelay; // 다음 공격까지 대기할 랜덤 시간 (ms)
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
    void UpdateAndDrawEnemies(SDL_Renderer* renderer, float playerX, float playerY, SDL_Texture* enemyTexture, float deltaTime);

    //투사체 충돌 체크 함수
    void CheckEnemyCollision(void* projectileArray);

#ifdef __cplusplus //C++에서 이 헤더를 포함해도 C 언어 함수 이름을 그대로 사용할 수 있도록 설정
}
#endif
