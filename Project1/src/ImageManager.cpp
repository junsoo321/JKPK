#include "ImageManager.hpp"
#include "MapData.h"

#include <SDL_image.h>
#include <iostream>
#include <string>

// 전역 변수 정의
SDL_Texture* gPlayerIdleTex  = nullptr;
SDL_Texture* gPlayerWalkTex  = nullptr;
SDL_Texture* gPlayerHurtTex  = nullptr;
SDL_Texture* gPlayerDeathTex = nullptr;
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

    gPlayerIdleTex  = LoadTexture(AssetPath("Characters/Default/idle.png"));
    gPlayerWalkTex  = LoadTexture(AssetPath("Characters/Default/walk.png"));
    gPlayerHurtTex  = LoadTexture(AssetPath("Characters/Default/hurt.png"));
    gPlayerDeathTex = LoadTexture(AssetPath("Characters/Default/death.png"));
    gProjectileTexture      = LoadTexture(AssetPath("projectile/semi-colon.png"));

    gEnemyTexture           = LoadTexture(AssetPath("enemy.png"));
    gEnemyProjectileTexture = LoadTexture(AssetPath("attack.png"));

    gWallTexture            = LoadTexture(AssetPath("map/obstacle_book.png"));

    gMapTexture             = LoadTexture(AssetPath("map/map_main.png"));
    gBorderTexture          = LoadTexture(AssetPath("border.png"));

    gBossTexture = LoadTexture(AssetPath("enemy.png"));

}

void LoadCollisionMapsFromImages() {
    for (int p = 0; p < MAX_PATTERNS; p++) {
        std::string path = AssetPath(("map/map_collision_" + std::to_string(p) + ".png").c_str());

        SDL_Surface* surf = IMG_Load(path.c_str());
        if (!surf) continue;

        // 픽셀 접근을 위해 RGBA32로 변환
        SDL_Surface* img = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (!img) continue;

        SDL_LockSurface(img);
        Uint32* pixels = (Uint32*)img->pixels;
        int pitch = img->pitch / 4;

        // 이미지 해상도에 관계없이 타일 위치를 비율로 환산해서 샘플링
        float scaleX = (float)img->w / SCREEN_WIDTH;
        float scaleY = (float)img->h / SCREEN_HEIGHT;

        for (int r = 1; r < MAP_ROWS - 1; r++) {
            for (int c = 1; c < MAP_COLS - 1; c++) {
                int imgC = (int)((c * TILE_SIZE + TILE_SIZE / 2) * scaleX);
                int imgR = (int)((r * TILE_SIZE + TILE_SIZE / 2) * scaleY);
                if (imgC >= img->w) imgC = img->w - 1;
                if (imgR >= img->h) imgR = img->h - 1;

                Uint32 pixel = pixels[imgR * pitch + imgC];
                Uint8 red, green, blue, alpha;
                SDL_GetRGBA(pixel, img->format, &red, &green, &blue, &alpha);

                // 마젠타(255,0,255) → 1: 외곽벽(투명 장애물)
                // 시안  (0,255,255) → 2: 맵 장애물(텍스처 렌더링)
                // 노랑  (255,255,0) → 3: 문 통로(이동 가능)
                bool isMagenta = (red > 200 && green < 50  && blue > 200);
                bool isCyan    = (red < 50  && green > 200 && blue > 200);
                bool isYellow  = (red > 200 && green > 200 && blue < 50 );

                if      (isMagenta) mapLayouts[p][r][c] = 1;
                else if (isCyan)    mapLayouts[p][r][c] = 2;
                else if (isYellow)  mapLayouts[p][r][c] = 3;
                else                mapLayouts[p][r][c] = 0;
            }
        }

        SDL_UnlockSurface(img);
        SDL_FreeSurface(img);
    }
}

void FreeAllImages() {
    // 메모리 해제 (안전하게 nullptr 체크 후 해제)
    if (gPlayerIdleTex)     SDL_DestroyTexture(gPlayerIdleTex);
    if (gPlayerWalkTex)     SDL_DestroyTexture(gPlayerWalkTex);
    if (gPlayerHurtTex)     SDL_DestroyTexture(gPlayerHurtTex);
    if (gPlayerDeathTex)    SDL_DestroyTexture(gPlayerDeathTex);
    if (gWallTexture)       SDL_DestroyTexture(gWallTexture);
    if (gFloorTexture)      SDL_DestroyTexture(gFloorTexture);
    if (gEnemyTexture)      SDL_DestroyTexture(gEnemyTexture);
    if (gProjectileTexture) SDL_DestroyTexture(gProjectileTexture);
    if (gEnemyProjectileTexture) SDL_DestroyTexture(gEnemyProjectileTexture);
    if (gMapTexture)        SDL_DestroyTexture(gMapTexture);
    if (gBorderTexture)     SDL_DestroyTexture(gBorderTexture);

    IMG_Quit(); // SDL_image 종료
}
