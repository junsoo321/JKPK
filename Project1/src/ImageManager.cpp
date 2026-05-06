#include "ImageManager.hpp"
#include <stdio.h>
//이미지 불러오기 및 변수에 할당하는 .cpp파일, SDL 함수 구조를 몰라서 Gemini로 임시 생성함

//!!! 외부에서 사용할 수 있도록 변수를 정의합니다.
SDL_Texture* gPlayerTexture = nullptr;
SDL_Texture* gWallTexture = nullptr;
SDL_Texture* gFloorTexture = nullptr;

void LoadAllImages(SDL_Renderer* renderer) {
    //!!! [중요] 현재는 실제 파일이 없으므로 nullptr 상태입니다.
    //!!! 나중에 이미지가 준비되면 SDL_CreateTextureFromSurface 등을 사용해 교체하세요.

    // 예시: gPlayerTexture = IMG_LoadTexture(renderer, "assets/player.png");
    printf("Images Manager: Ready to load images.\n");
}

void FreeAllImages() {
    //!!! 사용한 메모리를 해제합니다.
    if (gPlayerTexture) SDL_DestroyTexture(gPlayerTexture);
    if (gWallTexture) SDL_DestroyTexture(gWallTexture);
    if (gFloorTexture) SDL_DestroyTexture(gFloorTexture);
}