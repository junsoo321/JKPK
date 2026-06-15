#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

// return current project execution directory
inline auto GetImageManagerSourcePath() -> const char* { return __FILE__; }

extern TTF_Font* gFont;

extern SDL_Texture* gPlayerIdleTex;		// characters/Default/idle.png
extern SDL_Texture* gPlayerWalkTex;		// characters/Default/walk.png
extern SDL_Texture* gPlayerHurtTex;		// characters/Default/hurt.png
extern SDL_Texture* gPlayerDeathTex;	// characters/Default/death.png
extern SDL_Texture* gWallTexture;		//!!! 벽 타일 외형 이미지 변수
extern SDL_Texture* gFloorTexture;		//!!! 바닥 타일 외형 이미지 변수
extern SDL_Texture* gObstacleTex;		// map/obstacle.png
extern SDL_Texture* gNormalEnemyTex;    // mob/normal.png
extern SDL_Texture* gNinjaEnemyTex;     // mob/ninja.png
extern SDL_Texture* gSuicideEnemyTex;   // mob/suicide.png
extern SDL_Texture* gProjectileTexture;
extern SDL_Texture* gEnemyProjectileTexture;  // mob/projectile/default.png  (texIndex 2)
extern SDL_Texture* gNinjaProjectileTex;       // mob/projectile/ninja.png    (texIndex 3)
extern SDL_Texture* gBossProjectileTex1;       // mob/projectile/boss-001.png (texIndex 0)
extern SDL_Texture* gBossProjectileTex2;       // mob/projectile/boss-002.png (texIndex 1)
extern SDL_Texture* gMapTexture;		//!!! 맵 배경 이미지 변수
extern SDL_Texture* gBossTexture;
extern SDL_Texture* gBossMapTexture;	// map/map_boss.png
extern SDL_Texture* gHeartFullTex;
extern SDL_Texture* gHeartHalfTex;
extern SDL_Texture* gHeartEmptyTex;
extern SDL_Texture* gButtonTex;
extern SDL_Texture* gTitleBgTex;
extern SDL_Texture* gComputerTex;    // map/quiz/computer_off.png
extern SDL_Texture* gComputerOnTex;  // map/quiz/computer_on.png
extern SDL_Texture* gMazeWallTex;
extern SDL_Texture* gMazeFloorTex;
extern SDL_Texture* gMazeArrowTex;
extern SDL_Texture* gMazePlayerIconTex[5];

auto LoadAllImages(SDL_Renderer* renderer) -> void;
void FreeAllImages();

// 보스 맵 collision (map/map_boss_collision.png → bossMapLayout)
void LoadCollisionMapsFromImages();

// 도어 마스크(U=1 D=2 L=4 R=8)에 맞는 배경 텍스처 반환 (합성 후 캐싱)
SDL_Texture* GetRoomMapTexture(SDL_Renderer* renderer, int doorMask);
// 클리어 상태 — 문이 열린 버전 배경 텍스처 반환 (합성 후 캐싱)
SDL_Texture* GetRoomMapTextureOpen(SDL_Renderer* renderer, int doorMask);

// 도어 마스크에 맞는 collision 생성 (meta PNG 기반)
void LoadRoomCollisionMap(int doorMask, int variant);

// 방향별 문 rect 반환 (0=U 1=D 2=L 3=R), 미로드 시 {0,0,0,0}
SDL_Rect GetDoorRect(int dir);
