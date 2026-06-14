#include "ImageManager.hpp"
#include "MapData.hpp"
#include "Constants.h"

#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <cstring>

// [Public textures]
SDL_Texture* gPlayerIdleTex          = nullptr;
SDL_Texture* gPlayerWalkTex          = nullptr;
SDL_Texture* gPlayerHurtTex          = nullptr;
SDL_Texture* gPlayerDeathTex         = nullptr;
SDL_Texture* gWallTexture            = nullptr;
SDL_Texture* gFloorTexture           = nullptr;
SDL_Texture* gEnemyTexture           = nullptr;
SDL_Texture* gProjectileTexture      = nullptr;
SDL_Texture* gEnemyProjectileTexture = nullptr;
SDL_Texture* gBossProjectileTex1     = nullptr;
SDL_Texture* gBossProjectileTex2     = nullptr;
SDL_Texture* gMapTexture             = nullptr;
SDL_Texture* gBossTexture            = nullptr;
SDL_Texture* gBossMapTexture         = nullptr;
SDL_Texture* gHeartFullTex           = nullptr;
SDL_Texture* gHeartHalfTex           = nullptr;
SDL_Texture* gHeartEmptyTex          = nullptr;
SDL_Texture* gButtonTex              = nullptr;
SDL_Texture* gTitleBgTex             = nullptr;
SDL_Texture* gMazeWallTex            = nullptr;
SDL_Texture* gMazeFloorTex           = nullptr;
TTF_Font*    gFont                   = nullptr;

// [Internal - map system]
static SDL_Texture* gMapBaseTex         = nullptr;
static SDL_Texture* gDoorTexU           = nullptr;
static SDL_Texture* gDoorTexD           = nullptr;
static SDL_Texture* gDoorTexL           = nullptr;
static SDL_Texture* gDoorTexR           = nullptr;
static SDL_Texture* gRoomMapTextures[16] = {};

// meta PNG parsed data
static int      metaCollision[MAP_ROWS][MAP_COLS] = {};
static SDL_Rect doorRects[4] = {};  // 0=U 1=D 2=L 3=R  (screen coords)
static bool     collisionReady = false;
static bool     doorsReady     = false;

// [Utilities]
static auto AssetPath(const char* filename) -> std::string {
    std::string path(GetImageManagerSourcePath());
    for (char& c : path) if (c == '\\') c = '/';
    size_t pos  = path.rfind('/');
    std::string hppDir    = (pos  != std::string::npos) ? path.substr(0, pos)      : ".";
    size_t pos2 = hppDir.rfind('/');
    std::string parentDir = (pos2 != std::string::npos) ? hppDir.substr(0, pos2+1) : "./";
    return parentDir + "assets/" + filename;
}

// meta PNG color rules
static bool IsMagenta(Uint8 r, Uint8 g, Uint8 b) { return r > 200 && g < 50  && b > 200; } // #FF00FF collision: wall
static bool IsYellow (Uint8 r, Uint8 g, Uint8 b) { return r > 200 && g > 200 && b < 50;  } // #FFFF00 collision: obstacle
static bool IsOrange (Uint8 r, Uint8 g, Uint8 b) { return r > 200 && g > 80  && g < 180 && b < 50; } // #FF8000 door U
static bool IsCyan   (Uint8 r, Uint8 g, Uint8 b) { return r < 50  && g > 200 && b > 200; } // #00FFFF door D
static bool IsLime   (Uint8 r, Uint8 g, Uint8 b) { return r > 80  && r < 180 && g > 200 && b < 50; } // #80FF00 door L
static bool IsPurple (Uint8 r, Uint8 g, Uint8 b) { return r > 80  && r < 180 && g < 50  && b > 200; } // #8000FF door R

// map_base_001_collision.png: 마젠타 → 벽 충돌 타일 (타일 중앙 샘플링)
static void ParseCollisionPNG(SDL_Surface* img) {
    SDL_LockSurface(img);
    Uint32* pixels = (Uint32*)img->pixels;
    int pitch = img->pitch / 4;

    memset(metaCollision, 0, sizeof(metaCollision));
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            int px = (int)((col * TILE_SIZE + TILE_SIZE / 2) * (float)img->w / SCREEN_WIDTH);
            int py = (int)((row * TILE_SIZE + TILE_SIZE / 2) * (float)img->h / SCREEN_HEIGHT);
            if (px >= img->w) px = img->w - 1;
            if (py >= img->h) py = img->h - 1;
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixels[py * pitch + px], img->format, &r, &g, &b, &a);
            if      (IsMagenta(r, g, b)) metaCollision[row][col] = 1;
            else if (IsYellow (r, g, b)) metaCollision[row][col] = 2;
            else                         metaCollision[row][col] = 0;
        }
    }

    SDL_UnlockSurface(img);
    collisionReady = true;
}

// map_base_001_doors.png: 오렌지/시안/라임/보라 영역 → 문 방향별 bounding box (화면 좌표)
static void ParseDoorsPNG(SDL_Surface* img) {
    SDL_LockSurface(img);
    Uint32* pixels = (Uint32*)img->pixels;
    int pitch = img->pitch / 4;

    int minX[4] = { img->w, img->w, img->w, img->w };
    int minY[4] = { img->h, img->h, img->h, img->h };
    int maxX[4] = { -1, -1, -1, -1 };
    int maxY[4] = { -1, -1, -1, -1 };

    for (int py = 0; py < img->h; py++) {
        for (int px = 0; px < img->w; px++) {
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixels[py * pitch + px], img->format, &r, &g, &b, &a);
            int dir = -1;
            if      (IsOrange(r, g, b)) dir = 0;
            else if (IsCyan  (r, g, b)) dir = 1;
            else if (IsLime  (r, g, b)) dir = 2;
            else if (IsPurple(r, g, b)) dir = 3;
            if (dir >= 0) {
                if (px < minX[dir]) minX[dir] = px;
                if (py < minY[dir]) minY[dir] = py;
                if (px > maxX[dir]) maxX[dir] = px;
                if (py > maxY[dir]) maxY[dir] = py;
            }
        }
    }

    SDL_UnlockSurface(img);

    float sx = (float)SCREEN_WIDTH  / img->w;
    float sy = (float)SCREEN_HEIGHT / img->h;
    for (int d = 0; d < 4; d++) {
        if (maxX[d] < 0) { doorRects[d] = {0, 0, 0, 0}; continue; }
        doorRects[d] = {
            (int)(minX[d] * sx),
            (int)(minY[d] * sy),
            (int)((maxX[d] - minX[d] + 1) * sx),
            (int)((maxY[d] - minY[d] + 1) * sy)
        };
    }
    doorsReady = true;
}

// tile center sampling for boss collision map (magenta only)
static void SampleCollisionImage(SDL_Surface* img, int out[][MAP_COLS]) {
    SDL_LockSurface(img);
    Uint32* pixels = (Uint32*)img->pixels;
    int pitch = img->pitch / 4;
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            int px = (int)((col * TILE_SIZE + TILE_SIZE / 2) * (float)img->w / SCREEN_WIDTH);
            int py = (int)((row * TILE_SIZE + TILE_SIZE / 2) * (float)img->h / SCREEN_HEIGHT);
            if (px >= img->w) px = img->w - 1;
            if (py >= img->h) py = img->h - 1;
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixels[py * pitch + px], img->format, &r, &g, &b, &a);
            out[row][col] = IsMagenta(r, g, b) ? 1 : 0;
        }
    }
    SDL_UnlockSurface(img);
}

// [Public]
auto LoadAllImages(SDL_Renderer* renderer) -> void {
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_image init failed: " << IMG_GetError() << std::endl;
        return;
    }

    auto LoadTex = [&](const std::string& path) -> SDL_Texture* {
        SDL_Texture* t = IMG_LoadTexture(renderer, path.c_str());
        if (!t) std::cout << "Load failed: " << path << " (" << IMG_GetError() << ")\n";
        return t;
    };

    // characters / effects
    gPlayerIdleTex          = LoadTex(AssetPath("Characters/Default/idle.png"));
    gPlayerWalkTex          = LoadTex(AssetPath("Characters/Default/walk.png"));
    gPlayerHurtTex          = LoadTex(AssetPath("Characters/Default/hurt.png"));
    gPlayerDeathTex         = LoadTex(AssetPath("Characters/Default/death.png"));
    gProjectileTexture      = LoadTex(AssetPath("projectile/semi-colon.png"));
    gEnemyTexture           = LoadTex(AssetPath("enemy.png"));
    gEnemyProjectileTexture = LoadTex(AssetPath("attack.png"));
    gBossProjectileTex1     = LoadTex(AssetPath("attack_boss_001.png"));
    gBossProjectileTex2     = LoadTex(AssetPath("attack_boss_002.png"));
    gBossTexture            = LoadTex(AssetPath("boss.png"));
    gBossMapTexture         = LoadTex(AssetPath("map/boss/map_boss.png"));
    gWallTexture            = LoadTex(AssetPath("transparent.png"));
    gMapTexture             = LoadTex(AssetPath("map/base/map_base_001.png")); // quiz fallback

    // heart UI
    gHeartFullTex  = LoadTex(AssetPath("display/heart_full.png"));
    gHeartHalfTex  = LoadTex(AssetPath("display/heart_half.png"));
    gHeartEmptyTex = LoadTex(AssetPath("display/heart_empty.png"));

    // UI
    gButtonTex    = LoadTex(AssetPath("ui/button.png"));
    gTitleBgTex   = LoadTex(AssetPath("ui/main.png"));

    // 미로
    gMazeWallTex  = LoadTex(AssetPath("maze/maze_wall.png"));
    gMazeFloorTex = LoadTex(AssetPath("maze/maze_floor.png"));

    // font
    gFont = TTF_OpenFont(AssetPath("Pretendard-Regular.ttf").c_str(), 24);
    if (!gFont) std::cout << "Font load failed: " << TTF_GetError() << "\n";

    // base map (map/base/map_base_001.png, fallback to map/map_base.png)
    gMapBaseTex = LoadTex(AssetPath("map/base/map_base_001.png"));
    if (!gMapBaseTex) gMapBaseTex = LoadTex(AssetPath("map/map_base.png"));

    // parse collision + door PNGs separately
    auto LoadAndParse = [](const std::string& path, void(*parseFn)(SDL_Surface*)) {
        SDL_Surface* surf = IMG_Load(path.c_str());
        if (!surf) { std::cout << "Meta not found: " << path << "\n"; return; }
        SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (rgba) { parseFn(rgba); SDL_FreeSurface(rgba); }
    };
    LoadAndParse(AssetPath("map/base/map_base_001_collision.png"), ParseCollisionPNG);
    LoadAndParse(AssetPath("map/base/map_base_001_doors.png"),     ParseDoorsPNG);

    // door sprites
    struct { const char* path; SDL_Texture** tex; } doors[4] = {
        { "map/door/map_door_U.png", &gDoorTexU },
        { "map/door/map_door_D.png", &gDoorTexD },
        { "map/door/map_door_L.png", &gDoorTexL },
        { "map/door/map_door_R.png", &gDoorTexR },
    };
    for (auto& d : doors)
        *d.tex = LoadTex(AssetPath(d.path));
}

// boss map collision (map_boss_collision.png → bossMapLayout)
void LoadCollisionMapsFromImages() {
    SDL_Surface* surf = IMG_Load(AssetPath("map/boss/map_boss_collision.png").c_str());
    if (surf) {
        SDL_Surface* img = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (img) { SampleCollisionImage(img, bossMapLayout); SDL_FreeSurface(img); }
    }
}

// build collision for doorMask: meta collision base + clear tiles in active door rects
void LoadRoomCollisionMap(int doorMask) {
    if (collisionLoaded[doorMask]) return;

    if (collisionReady)
        memcpy(mapLayouts[doorMask], metaCollision, sizeof(metaCollision));
    else
        memset(mapLayouts[doorMask], 0, sizeof(mapLayouts[doorMask]));

    for (int d = 0; d < 4; d++) {
        if (!(doorMask & (1 << d))) continue;
        SDL_Rect& rect = doorRects[d];
        if (rect.w == 0 || rect.h == 0) continue;
        int colMin = rect.x / TILE_SIZE;
        int colMax = (rect.x + rect.w - 1) / TILE_SIZE;
        int rowMin = rect.y / TILE_SIZE;
        int rowMax = (rect.y + rect.h - 1) / TILE_SIZE;
        for (int row = rowMin; row <= rowMax && row < MAP_ROWS; row++)
            for (int col = colMin; col <= colMax && col < MAP_COLS; col++)
                mapLayouts[doorMask][row][col] = 0;
    }

    collisionLoaded[doorMask] = true;
}

// compose base + door sprites into a cached render-target texture
SDL_Texture* GetRoomMapTexture(SDL_Renderer* renderer, int doorMask) {
    if (gRoomMapTextures[doorMask]) return gRoomMapTextures[doorMask];

    SDL_Texture* composed = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!composed) return nullptr;

    SDL_SetTextureBlendMode(composed, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, composed);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (gMapBaseTex) SDL_RenderCopy(renderer, gMapBaseTex, NULL, NULL);

    SDL_Texture* doorTexs[4] = { gDoorTexU, gDoorTexD, gDoorTexL, gDoorTexR };
    for (int d = 0; d < 4; d++) {
        if (!(doorMask & (1 << d)) || !doorTexs[d]) continue;
        SDL_Rect dst = doorRects[d];
        if (dst.w == 0 || dst.h == 0) continue;
        SDL_SetTextureBlendMode(doorTexs[d], SDL_BLENDMODE_NONE);
        SDL_RenderCopy(renderer, doorTexs[d], NULL, &dst);
    }

    SDL_SetRenderTarget(renderer, NULL);
    gRoomMapTextures[doorMask] = composed;
    return composed;
}

SDL_Rect GetDoorRect(int dir) {
    return (dir >= 0 && dir < 4) ? doorRects[dir] : SDL_Rect{0, 0, 0, 0};
}

void FreeAllImages() {
    if (gFont) { TTF_CloseFont(gFont); gFont = nullptr; }

    for (int i = 0; i < 16; i++) {
        if (gRoomMapTextures[i]) SDL_DestroyTexture(gRoomMapTextures[i]);
        gRoomMapTextures[i] = nullptr;
    }

    auto Free = [](SDL_Texture*& t) { if (t) { SDL_DestroyTexture(t); t = nullptr; } };
    Free(gPlayerIdleTex);
    Free(gPlayerWalkTex);
    Free(gPlayerHurtTex);
    Free(gPlayerDeathTex);
    Free(gWallTexture);
    Free(gFloorTexture);
    Free(gEnemyTexture);
    Free(gProjectileTexture);
    Free(gEnemyProjectileTexture);
    Free(gBossProjectileTex1);
    Free(gBossProjectileTex2);
    Free(gMapTexture);
    Free(gBossTexture);
    Free(gBossMapTexture);
    Free(gHeartFullTex);
    Free(gHeartHalfTex);
    Free(gHeartEmptyTex);
    Free(gButtonTex);
    Free(gTitleBgTex);
    Free(gMazeWallTex);
    Free(gMazeFloorTex);
    Free(gMapBaseTex);
    Free(gDoorTexU);
    Free(gDoorTexD);
    Free(gDoorTexL);
    Free(gDoorTexR);

    IMG_Quit();
}
