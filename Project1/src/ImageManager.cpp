#include "ImageManager.hpp"
#include <SDL_image.h> // SDL_image 라이브러리 필요
#include <stdio.h>

// 전역 변수 정의
SDL_Texture* gPlayerTexture = nullptr;
SDL_Texture* gWallTexture = nullptr;
SDL_Texture* gFloorTexture = nullptr;
SDL_Texture* gEnemyTexture = nullptr;
SDL_Texture* gProjectileTexture = nullptr;
SDL_Texture* gEnemyProjectileTexture = nullptr;

void LoadAllImages(SDL_Renderer* renderer) {
    // 1. SDL_image 초기화 (PNG 로드 설정)
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return;
    }

    // 2. 이미지 로드 함수 (내부 유틸리티)
    auto LoadTexture = [&](const char* path) -> SDL_Texture* {
        SDL_Texture* newTexture = IMG_LoadTexture(renderer, path);
        if (newTexture == nullptr) {
            printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        }
        return newTexture;
        };

    // 3. 실제 파일 로드 (경로와 파일명은 본인의 환경에 맞게 수정하세요)
    gProjectileTexture = LoadTexture("C:/Users/junso/source/repos/Project1/Project1/assets/attack.png"); // 투사체 이미지 로드
    gPlayerTexture = LoadTexture("C:/Users/junso/source/repos/Project1/Project1/assets/enemy.png");
    gEnemyTexture = LoadTexture("C:/Users/junso/source/repos/Project1/Project1/assets/player.png");
    gEnemyProjectileTexture = LoadTexture("C:/Users/junso/source/repos/Project1/Project1/assets/attack.png");

    printf("Images Manager: All images loaded successfully.\n");
}

void FreeAllImages() {
    // 메모리 해제 (안전하게 nullptr 체크 후 해제)
    if (gPlayerTexture)     SDL_DestroyTexture(gPlayerTexture);
    if (gWallTexture)       SDL_DestroyTexture(gWallTexture);
    if (gFloorTexture)      SDL_DestroyTexture(gFloorTexture);
    if  (gEnemyTexture)      SDL_DestroyTexture(gEnemyTexture);
    if (gProjectileTexture) SDL_DestroyTexture(gProjectileTexture);
    if (gEnemyProjectileTexture) SDL_DestroyTexture(gEnemyProjectileTexture);

    IMG_Quit(); // SDL_image 종료
}