#include "ImageManager.hpp"

#include <SDL_image.h>
#include <iostream>
#include <string>

// 전역 변수 정의
SDL_Texture* gPlayerTexture = nullptr;
SDL_Texture* gWallTexture = nullptr;
SDL_Texture* gFloorTexture = nullptr;
SDL_Texture* gEnemyTexture = nullptr;
SDL_Texture* gProjectileTexture = nullptr;
SDL_Texture* gEnemyProjectileTexture = nullptr;
SDL_Texture* gMapTexture = nullptr;
SDL_Texture* gBorderTexture = nullptr;
SDL_Texture* gBossTexture = nullptr;

// ImageManager.hpp 상위 디렉터리의 assets/ 폴더를 기준으로 경로 반환
static auto AssetPath(const char* filename) -> std::string
{
    std::string path(GetImageManagerSourcePath());
    for (char& c : path) if (c == '\\') c = '/';

    // 파일명 제거 → include/ 디렉터리
    size_t pos = path.rfind('/');
    std::string hppDir = (pos != std::string::npos) ? path.substr(0, pos) : ".";

    // 한 단계 상위 디렉터리로 이동
    size_t pos2 = hppDir.rfind('/');
    std::string parentDir = (pos2 != std::string::npos) ? hppDir.substr(0, pos2 + 1) : "./";
    return parentDir + "assets/" + filename;
}

auto LoadAllImages(SDL_Renderer* renderer) -> void
{
    // 1. SDL_image 초기화 (PNG 로드 설정)
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return;
    }

    // 2. 이미지 로드 함수 (내부 유틸리티)
    auto LoadTexture = [&renderer](const std::string& path) -> SDL_Texture* {
        SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
        if (newTexture == nullptr)
        {
            std::cout   << "Unable to Load Image\t: "   << path << "\n"
                        << "SDL_image Error\t: "        << IMG_GetError() << std::endl;
        }
        return newTexture;
    };

    // 3. 실제 파일 로드

    gPlayerTexture          = LoadTexture(AssetPath("player.png"));
    gProjectileTexture      = LoadTexture(AssetPath("attack.png"));

    gEnemyTexture           = LoadTexture(AssetPath("enemy.png"));
    gEnemyProjectileTexture = LoadTexture(AssetPath("attack.png"));

    gWallTexture            = LoadTexture(AssetPath("obstacle_book.png"));

    gMapTexture             = LoadTexture(AssetPath("map_main.png"));
    gBorderTexture          = LoadTexture(AssetPath("border.png"));

    gBossTexture = LoadTexture(AssetPath("enemy.png"));

    // 3. 실제 파일 로드 (경로와 파일명은 본인의 환경에 맞게 수정하세요)
    std::cout << "Images Manager: All images loaded successfully." << std::endl;
}

void FreeAllImages() {
    // 메모리 해제 (안전하게 nullptr 체크 후 해제)
    if (gPlayerTexture)     SDL_DestroyTexture(gPlayerTexture);
    if (gWallTexture)       SDL_DestroyTexture(gWallTexture);
    if (gFloorTexture)      SDL_DestroyTexture(gFloorTexture);
    if (gEnemyTexture)      SDL_DestroyTexture(gEnemyTexture);
    if (gProjectileTexture) SDL_DestroyTexture(gProjectileTexture);
    if (gEnemyProjectileTexture) SDL_DestroyTexture(gEnemyProjectileTexture);
    if (gMapTexture)        SDL_DestroyTexture(gMapTexture);
    if (gBorderTexture)     SDL_DestroyTexture(gBorderTexture);

    IMG_Quit(); // SDL_image 종료
}
