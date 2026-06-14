#include "ImageManager.hpp"
#include "MapData.hpp"
#include "Constants.h"

#include <SDL_image.h>
#include <iostream>
#include <string>
#include <cstring>

// [Textures]
SDL_Texture* gRoomMapTextures[16] = {};

SDL_Texture* gPlayerIdleTex  = nullptr;
SDL_Texture* gPlayerWalkTex  = nullptr;
SDL_Texture* gPlayerHurtTex  = nullptr;
SDL_Texture* gPlayerDeathTex = nullptr;
SDL_Texture* gWallTexture    = nullptr;
SDL_Texture* gFloorTexture   = nullptr;
SDL_Texture* gEnemyTexture   = nullptr;
SDL_Texture* gProjectileTexture      = nullptr;
SDL_Texture* gEnemyProjectileTexture = nullptr;
SDL_Texture* gBossProjectileTex1     = nullptr;
SDL_Texture* gBossProjectileTex2     = nullptr;
SDL_Texture* gMapTexture     = nullptr;
SDL_Texture* gBossTexture    = nullptr;
SDL_Texture* gBossMapTexture = nullptr;
SDL_Texture* gHeartFullTex   = nullptr;
SDL_Texture* gHeartHalfTex   = nullptr;
SDL_Texture* gHeartEmptyTex  = nullptr;

// base map + per-direction door overlay textures
SDL_Texture* gMapBaseTex = nullptr;
SDL_Texture* gDoorTexU   = nullptr;
SDL_Texture* gDoorTexD   = nullptr;
SDL_Texture* gDoorTexL   = nullptr;
SDL_Texture* gDoorTexR   = nullptr;

// doorClear[d][r][c] = 1 if door overlay d covers tile (r,c) -> clear collision
static int  doorClear[4][MAP_ROWS][MAP_COLS] = {};
static bool doorClearReady = false;

// baseCollision: loaded from map_collision_base.png
static int  baseCollision[MAP_ROWS][MAP_COLS] = {};
static bool baseCollisionReady = false;

// [Utilities]
static auto AssetPath(const char* filename) -> std::string
{
    std::string path(GetImageManagerSourcePath());
    for (char& c : path) if (c == '\\') c = '/';
    size_t pos  = path.rfind('/');
    std::string hppDir    = (pos  != std::string::npos) ? path.substr(0, pos)      : ".";
    size_t pos2 = hppDir.rfind('/');
    std::string parentDir = (pos2 != std::string::npos) ? hppDir.substr(0, pos2+1) : "./";
    return parentDir + "assets/" + filename;
}

// pixel color -> tile type (collision)
static int ColorToTile(Uint8 r, Uint8 g, Uint8 b) {
    if (r > 200 && g < 50  && b > 200) return 1; // magenta -> outer wall
    if (r < 50  && g > 200 && b > 200) return 2; // cyan    -> obstacle
    if (r > 200 && g > 200 && b < 50 ) return 3; // yellow  -> door (passable)
    return 0;
}

// sample color-coded collision PNG into out[][]
static void SampleCollisionImage(SDL_Surface* img, int out[][MAP_COLS]) {
    SDL_LockSurface(img);
    Uint32* pixels = (Uint32*)img->pixels;
    int pitch = img->pitch / 4;
    float sx = (float)img->w / SCREEN_WIDTH;
    float sy = (float)img->h / SCREEN_HEIGHT;
    for (int r = 1; r < MAP_ROWS - 1; r++) {
        for (int c = 1; c < MAP_COLS - 1; c++) {
            int px = (int)((c * TILE_SIZE + TILE_SIZE / 2) * sx);
            int py = (int)((r * TILE_SIZE + TILE_SIZE / 2) * sy);
            if (px >= img->w) px = img->w - 1;
            if (py >= img->h) py = img->h - 1;
            Uint8 red, green, blue, alpha;
            SDL_GetRGBA(pixels[py * pitch + px], img->format, &red, &green, &blue, &alpha);
            out[r][c] = ColorToTile(red, green, blue);
        }
    }
    SDL_UnlockSurface(img);
}

// sample door overlay alpha channel: non-transparent tiles are recorded in doorClear[dirIdx]
static void SampleDoorOverlay(SDL_Surface* img, int dirIdx) {
    SDL_LockSurface(img);
    Uint32* pixels = (Uint32*)img->pixels;
    int pitch = img->pitch / 4;
    float sx = (float)img->w / SCREEN_WIDTH;
    float sy = (float)img->h / SCREEN_HEIGHT;
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int px = (int)((c * TILE_SIZE + TILE_SIZE / 2) * sx);
            int py = (int)((r * TILE_SIZE + TILE_SIZE / 2) * sy);
            if (px >= img->w) px = img->w - 1;
            if (py >= img->h) py = img->h - 1;
            Uint8 red, green, blue, alpha;
            SDL_GetRGBA(pixels[py * pitch + px], img->format, &red, &green, &blue, &alpha);
            doorClear[dirIdx][r][c] = (alpha > 128) ? 1 : 0;
        }
    }
    SDL_UnlockSurface(img);
}

// [Public]
auto LoadAllImages(SDL_Renderer* renderer) -> void
{
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
    gPlayerIdleTex       = LoadTex(AssetPath("Characters/Default/idle.png"));
    gPlayerWalkTex       = LoadTex(AssetPath("Characters/Default/walk.png"));
    gPlayerHurtTex       = LoadTex(AssetPath("Characters/Default/hurt.png"));
    gPlayerDeathTex      = LoadTex(AssetPath("Characters/Default/death.png"));
    gProjectileTexture       = LoadTex(AssetPath("projectile/semi-colon.png"));
    gEnemyTexture            = LoadTex(AssetPath("enemy.png"));
    gEnemyProjectileTexture  = LoadTex(AssetPath("attack.png"));
    gBossProjectileTex1      = LoadTex(AssetPath("attack_boss_001.png"));
    gBossProjectileTex2      = LoadTex(AssetPath("attack_boss_002.png"));
    gBossTexture             = LoadTex(AssetPath("boss.png"));
    gBossMapTexture          = LoadTex(AssetPath("map/map_boss.png"));
    gWallTexture             = LoadTex(AssetPath("transparent.png"));
    gMapTexture              = LoadTex(AssetPath("map/map_main.png")); // fallback

    // heart UI
    gHeartFullTex   = LoadTex(AssetPath("display/heart_full.png"));
    gHeartHalfTex   = LoadTex(AssetPath("display/heart_half.png"));
    gHeartEmptyTex  = LoadTex(AssetPath("display/heart_empty.png"));

    // base map + door overlays
    gMapBaseTex = LoadTex(AssetPath("map/map_base.png"));

    // order: 0=U 1=D 2=L 3=R (matches door mask bits)
    const char* doorFiles[4] = {
        "map/map_door_U.png",
        "map/map_door_D.png",
        "map/map_door_L.png",
        "map/map_door_R.png"
    };
    SDL_Texture** doorTexPtrs[4] = { &gDoorTexU, &gDoorTexD, &gDoorTexL, &gDoorTexR };

    for (int d = 0; d < 4; d++) {
        SDL_Surface* surf = IMG_Load(AssetPath(doorFiles[d]).c_str());
        if (!surf) {
            std::cout << "Door overlay not found: " << doorFiles[d] << "\n";
            continue;
        }
        SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (!rgba) continue;

        SampleDoorOverlay(rgba, d);

        *doorTexPtrs[d] = SDL_CreateTextureFromSurface(renderer, rgba);
        if (*doorTexPtrs[d])
            SDL_SetTextureBlendMode(*doorTexPtrs[d], SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(rgba);
    }
    doorClearReady = true;

    // base collision map
    SDL_Surface* csurf = IMG_Load(AssetPath("map/map_collision_base.png").c_str());
    if (csurf) {
        SDL_Surface* crgba = SDL_ConvertSurfaceFormat(csurf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(csurf);
        if (crgba) {
            SampleCollisionImage(crgba, baseCollision);
            SDL_FreeSurface(crgba);
            baseCollisionReady = true;
        }
    }
}

// boss map collision only
void LoadCollisionMapsFromImages() {
    SDL_Surface* surf = IMG_Load(AssetPath("map/map_boss_collision.png").c_str());
    if (surf) {
        SDL_Surface* img = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (img) { SampleCollisionImage(img, bossMapLayout); SDL_FreeSurface(img); }
    }
}

// build collision for doorMask: copy base then clear tiles covered by active door overlays
void LoadRoomCollisionMap(int doorMask) {
    if (collisionLoaded[doorMask]) return;

    if (baseCollisionReady)
        memcpy(mapLayouts[doorMask], baseCollision, sizeof(baseCollision));
    else
        memset(mapLayouts[doorMask], 0, sizeof(mapLayouts[doorMask]));

    if (doorClearReady) {
        for (int d = 0; d < 4; d++) {
            if (!(doorMask & (1 << d))) continue;
            for (int r = 0; r < MAP_ROWS; r++)
                for (int c = 0; c < MAP_COLS; c++)
                    if (doorClear[d][r][c])
                        mapLayouts[doorMask][r][c] = 0;
        }
    }

    collisionLoaded[doorMask] = true;
}

// compose base + door overlays into a cached render-target texture
SDL_Texture* GetRoomMapTexture(SDL_Renderer* renderer, int doorMask) {
    if (gRoomMapTextures[doorMask]) return gRoomMapTextures[doorMask];

    SDL_Texture* composed = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!composed) return gMapTexture;

    SDL_SetTextureBlendMode(composed, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, composed);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_Texture* base = gMapBaseTex ? gMapBaseTex : gMapTexture;
    if (base) SDL_RenderCopy(renderer, base, NULL, NULL);

    SDL_Texture* doorTexs[4] = { gDoorTexU, gDoorTexD, gDoorTexL, gDoorTexR };
    for (int d = 0; d < 4; d++)
        if ((doorMask & (1 << d)) && doorTexs[d])
            SDL_RenderCopy(renderer, doorTexs[d], NULL, NULL);

    SDL_SetRenderTarget(renderer, NULL);

    gRoomMapTextures[doorMask] = composed;
    return composed;
}

void FreeAllImages() {
    for (int i = 0; i < 16; i++) {
        if (gRoomMapTextures[i] && gRoomMapTextures[i] != gMapTexture)
            SDL_DestroyTexture(gRoomMapTextures[i]);
        gRoomMapTextures[i] = nullptr;
    }

    if (gPlayerIdleTex)     { SDL_DestroyTexture(gPlayerIdleTex);     gPlayerIdleTex     = nullptr; }
    if (gPlayerWalkTex)     { SDL_DestroyTexture(gPlayerWalkTex);     gPlayerWalkTex     = nullptr; }
    if (gPlayerHurtTex)     { SDL_DestroyTexture(gPlayerHurtTex);     gPlayerHurtTex     = nullptr; }
    if (gPlayerDeathTex)    { SDL_DestroyTexture(gPlayerDeathTex);    gPlayerDeathTex    = nullptr; }
    if (gWallTexture)       { SDL_DestroyTexture(gWallTexture);       gWallTexture       = nullptr; }
    if (gFloorTexture)      { SDL_DestroyTexture(gFloorTexture);      gFloorTexture      = nullptr; }
    if (gEnemyTexture)      { SDL_DestroyTexture(gEnemyTexture);      gEnemyTexture      = nullptr; }
    if (gProjectileTexture) { SDL_DestroyTexture(gProjectileTexture); gProjectileTexture = nullptr; }
    if (gEnemyProjectileTexture) { SDL_DestroyTexture(gEnemyProjectileTexture); gEnemyProjectileTexture = nullptr; }
    if (gBossProjectileTex1)     { SDL_DestroyTexture(gBossProjectileTex1);     gBossProjectileTex1     = nullptr; }
    if (gBossProjectileTex2)     { SDL_DestroyTexture(gBossProjectileTex2);     gBossProjectileTex2     = nullptr; }
    if (gMapTexture)        { SDL_DestroyTexture(gMapTexture);        gMapTexture        = nullptr; }
    if (gBossTexture)       { SDL_DestroyTexture(gBossTexture);       gBossTexture       = nullptr; }
    if (gBossMapTexture)    { SDL_DestroyTexture(gBossMapTexture);    gBossMapTexture    = nullptr; }
    if (gHeartFullTex)      { SDL_DestroyTexture(gHeartFullTex);      gHeartFullTex      = nullptr; }
    if (gHeartHalfTex)      { SDL_DestroyTexture(gHeartHalfTex);      gHeartHalfTex      = nullptr; }
    if (gHeartEmptyTex)     { SDL_DestroyTexture(gHeartEmptyTex);     gHeartEmptyTex     = nullptr; }
    if (gMapBaseTex)        { SDL_DestroyTexture(gMapBaseTex);        gMapBaseTex        = nullptr; }
    if (gDoorTexU)          { SDL_DestroyTexture(gDoorTexU);          gDoorTexU          = nullptr; }
    if (gDoorTexD)          { SDL_DestroyTexture(gDoorTexD);          gDoorTexD          = nullptr; }
    if (gDoorTexL)          { SDL_DestroyTexture(gDoorTexL);          gDoorTexL          = nullptr; }
    if (gDoorTexR)          { SDL_DestroyTexture(gDoorTexR);          gDoorTexR          = nullptr; }

    IMG_Quit();
}
