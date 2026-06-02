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
    int   hp;
    bool  isInvincible;
    Uint32 invincibleEndTime;
    SDL_Rect drawRect;

    // 애니메이션
    int   animDir;      // 0=아래, 1=위, 2=옆
    bool  facingRight;  // 옆 방향일 때 좌우 반전 여부
    int   animFrame;
    float animTimer;
    float hurtTimer;    // > 0 이면 hurt 애니메이션 재생
    bool  isDead;
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
