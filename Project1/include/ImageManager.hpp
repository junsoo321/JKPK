#pragma once

//이미지 불러오기 및 변수에 할당하는 .hpp파일, SDL 함수 구조를 몰라서 Gemini로 임시 생성함
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

// return current project execution directory
inline auto GetImageManagerSourcePath() -> const char* { return __FILE__; }

extern TTF_Font* gFont;

//!!! 이미지를 저장하는 포인터 변수들입니다.
//!!! 나중에 실제 이미지로 교체할 때 이 변수들에 로드하면 됩니다.
extern SDL_Texture* gPlayerIdleTex;  // characters/Default/idle.png
extern SDL_Texture* gPlayerWalkTex;  // characters/Default/walk.png
extern SDL_Texture* gPlayerHurtTex;  // characters/Default/hurt.png
extern SDL_Texture* gPlayerDeathTex; // characters/Default/death.png
extern SDL_Texture* gWallTexture;   //!!! 벽 타일 외형 이미지 변수
extern SDL_Texture* gFloorTexture;  //!!! 바닥 타일 외형 이미지 변수
extern SDL_Texture* gEnemyTexture;
extern SDL_Texture* gProjectileTexture;
extern SDL_Texture* gEnemyProjectileTexture;
extern SDL_Texture* gBossProjectileTex1;
extern SDL_Texture* gBossProjectileTex2;
extern SDL_Texture* gMapTexture;    //!!! 맵 배경 이미지 변수
extern SDL_Texture* gBossTexture;
extern SDL_Texture* gBossMapTexture; // map/map_boss.png
extern SDL_Texture* gHeartFullTex;
extern SDL_Texture* gHeartHalfTex;
extern SDL_Texture* gHeartEmptyTex;
extern SDL_Texture* gButtonTex;
extern SDL_Texture* gTitleBgTex;
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

// 도어 마스크에 맞는 collision 생성 (meta PNG 기반)
void LoadRoomCollisionMap(int doorMask);

// 방향별 문 rect 반환 (0=U 1=D 2=L 3=R), 미로드 시 {0,0,0,0}
SDL_Rect GetDoorRect(int dir);
