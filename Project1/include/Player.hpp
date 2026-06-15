#pragma once

#include <SDL.h>
#include "Constants.h"

//플레이어 구조체
typedef struct PlayerData {
    float x;
    float y;
    float vx;
    float vy;
    float speed;
    float projectileSpeedMult;
    bool  hasCigarette;         // 공격 주기 0.5~1.5배 주기 변동
    bool  hasGlasses;           // 일반 몬스터 투사체 크기 1.5배
    int   hp;
    bool  isInvincible;
    Uint32 invincibleEndTime;
    SDL_Rect drawRect;

    // 애니메이션
    int   animDir;      // 0=앞(아래/row0), 1=옆(row1), 2=뒤(위/row2)
    bool  facingRight;  // 옆 방향일 때 좌우 반전 여부
    int   animFrame;
    float animTimer;
    float hurtTimer;    // > 0 이면 hurt 애니메이션 재생
    float healTimer;    // > 0 이면 초록 깜빡임 재생
    bool  isDead;

    // 아이템 인벤토리
    int collectedItems[MAX_INVENTORY];
    int itemCount;
} PlayerData;

#ifdef __cplusplus
extern "C" {
#endif
    void InitPlayer(PlayerData* p);
    void UpdatePlayer(PlayerData* p, const Uint8* keyboardState, float deltaTime);
    void DrawPlayer(SDL_Renderer* renderer, PlayerData* p);
    void DrawHearts(SDL_Renderer* renderer, PlayerData* p);
    bool CheckCollision(PlayerData* p, void* bulletArray, void* bossData);

#ifdef __cplusplus
}
#endif
