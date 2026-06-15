#pragma once

#include <SDL.h>
#include <stdbool.h>
#include "Constants.h"

//탄환 구조체
typedef struct {
    float x, y;
    float dirX, dirY;
    float angle;     //회전 각도 (도 단위)
    float speedMult; //투사체 속도 배율 (기본 1.0f, 아이템으로 증가)
    bool active;
    int owner;    //발사 대상 정보 (플레이어 : 0, 몹 : 1)
    int type;
    int texIndex; //보스 투사체 텍스처 인덱스 (0 또는 1, 교대 사용)
} Projectile;

#ifdef __cplusplus
extern "C" {
#endif
    extern Projectile bullets[MAX_PROJECTILES]; //투사체 정보 저장 배열(외부에서 참조 허용)

    void InitProjectiles(); //투사체 초기화
    void FireProjectile(float startX, float startY, float deltaTime, float speedMult); //플레이어 투사체 발사
    void FireEnemyProjectile(float startX, float startY, float targetX, float targetY);                          //일반 몹 투사체 (texIndex=2)
    void FireEnemyProjectileEx(float startX, float startY, float targetX, float targetY, int texIndex); //texIndex 지정 몹 투사체
    void FireBossProjectile(float startX, float startY, float targetX, float targetY, int texIndex);             //보스 투사체
    void UpdateAndDrawProjectiles(SDL_Renderer* renderer, float deltaTime, bool glassesActive); //투사체 출력 및 충돌 처리
    void FireProjectile_PHASE3(float startX, float startY, float targetX, float targetY, float speed);

#ifdef __cplusplus
}
#endif
