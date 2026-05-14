#pragma once

//이미지 불러오기 및 변수에 할당하는 .hpp파일, SDL 함수 구조를 몰라서 Gemini로 임시 생성함

#include <SDL.h>

//!!! 이미지를 저장하는 포인터 변수들입니다.
//!!! 나중에 실제 이미지로 교체할 때 이 변수들에 로드하면 됩니다.
extern SDL_Texture* gPlayerTexture; //!!! 플레이어 외형 이미지 변수
extern SDL_Texture* gWallTexture;   //!!! 벽 타일 외형 이미지 변수
extern SDL_Texture* gFloorTexture;  //!!! 바닥 타일 외형 이미지 변수
extern SDL_Texture* gEnemyTexture;
extern SDL_Texture* gProjectileTexture;
extern SDL_Texture* gEnemyProjectileTexture;
extern SDL_Texture* gBossTexture;
//!!! 이미지들을 불러오고 메모리에서 해제하는 함수들입니다.
void LoadAllImages(SDL_Renderer* renderer);
void FreeAllImages();
