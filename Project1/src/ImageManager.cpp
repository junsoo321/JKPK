#include "ImageManager.hpp"
#include "MapData.h"
#include "Constants.h"

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
SDL_Texture* gBossProjectileTex1 = nullptr;
SDL_Texture* gBossProjectileTex2 = nullptr;
SDL_Texture* gMapTexture = nullptr;
SDL_Texture* gBossTexture = nullptr;
SDL_Texture* gBossMapTexture = nullptr;
SDL_Texture* gHeartFullTex  = nullptr;
SDL_Texture* gHeartHalfTex  = nullptr;
SDL_Texture* gHeartEmptyTex = nullptr;

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
    gBossProjectileTex1     = LoadTexture(AssetPath("attack_boss_001.png"));
    gBossProjectileTex2     = LoadTexture(AssetPath("attack_boss_002.png"));

    gWallTexture            = LoadTexture(AssetPath("map/obstacle_book.png"));

    gMapTexture             = LoadTexture(AssetPath("map/map_main.png"));

    gBossTexture            = LoadTexture(AssetPath("boss.png"));
    gBossMapTexture         = LoadTexture(AssetPath("map/map_boss.png"));

    gHeartFullTex           = LoadTexture(AssetPath("display/heart_full.png"));
    gHeartHalfTex           = LoadTexture(AssetPath("display/heart_half.png"));
    gHeartEmptyTex          = LoadTexture(AssetPath("display/heart_empty.png"));

}

// 색상 픽셀 → 타일값 변환 공통 함수
static int ColorToTile(Uint8 red, Uint8 green, Uint8 blue) {
    if (red > 200 && green < 50  && blue > 200) return 1; // 마젠타 → 외곽벽
    if (red < 50  && green > 200 && blue > 200) return 2; // 시안   → 장애물
    if (red > 200 && green > 200 && blue < 50 ) return 3; // 노랑   → 문
    return 0;
}

static void SampleCollisionImage(SDL_Surface* img, int out[][MAP_COLS]) {
    SDL_LockSurface(img);
    Uint32* pixels = (Uint32*)img->pixels;
    int pitch = img->pitch / 4;
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
            out[r][c] = ColorToTile(red, green, blue);
        }
    }
    SDL_UnlockSurface(img);
}

void LoadCollisionMapsFromImages() {
    // 일반 맵 패턴 로드
    for (int p = 0; p < MAX_PATTERNS; p++) {
        std::string path = AssetPath(("map/map_collision_" + std::to_string(p) + ".png").c_str());
        SDL_Surface* surf = IMG_Load(path.c_str());
        if (!surf) continue;
        SDL_Surface* img = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (!img) continue;
        SampleCollisionImage(img, mapLayouts[p]);
        SDL_FreeSurface(img);
    }

    // 보스 맵 collision 로드
    {
        SDL_Surface* surf = IMG_Load(AssetPath("map/map_boss_collision.png").c_str());
        if (surf) {
            SDL_Surface* img = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(surf);
            if (img) {
                SampleCollisionImage(img, bossMapLayout);
                SDL_FreeSurface(img);
            }
        }
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
    if (gBossProjectileTex1)     SDL_DestroyTexture(gBossProjectileTex1);
    if (gBossProjectileTex2)     SDL_DestroyTexture(gBossProjectileTex2);
    if (gMapTexture)        SDL_DestroyTexture(gMapTexture);
    if (gBossMapTexture)    SDL_DestroyTexture(gBossMapTexture);
    if (gHeartFullTex)      SDL_DestroyTexture(gHeartFullTex);
    if (gHeartHalfTex)      SDL_DestroyTexture(gHeartHalfTex);
    if (gHeartEmptyTex)     SDL_DestroyTexture(gHeartEmptyTex);

    IMG_Quit(); // SDL_image 종료
}
