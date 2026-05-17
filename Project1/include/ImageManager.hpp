#pragma once

//이미지 불러오기 및 변수에 할당하는 .hpp파일, SDL 함수 구조를 몰라서 Gemini로 임시 생성함
#include <SDL.h>

// return current project execution directory
inline auto GetImageManagerSourcePath() -> const char* { return __FILE__; }

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
auto LoadAllImages(SDL_Renderer* renderer) -> void;
void FreeAllImages();

// assets/map_collision_N.png 파일을 읽어 mapLayouts[N] 덮어쓰기
// assets/map_boss_collision.png 파일을 읽어 bossMapLayout 덮어쓰기
// 파일이 없으면 기존 하드코딩 데이터 유지
void LoadCollisionMapsFromImages();
