#include "ImageManager.hpp"
#include "MapData.hpp"
#include "Constants.h"

#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <cstring>

// [Public textures]
SDL_Texture* gPlayerIdleTex             = nullptr;
SDL_Texture* gPlayerWalkTex             = nullptr;
SDL_Texture* gPlayerHurtTex             = nullptr;
SDL_Texture* gPlayerDeathTex            = nullptr;
SDL_Texture* gWallTexture               = nullptr;
SDL_Texture* gFloorTexture              = nullptr;
SDL_Texture* gObstacleTex              = nullptr;
SDL_Texture* gNormalEnemyTex           = nullptr;
SDL_Texture* gNinjaEnemyTex            = nullptr;
SDL_Texture* gSuicideEnemyTex          = nullptr;
SDL_Texture* gProjectileTexture         = nullptr;
SDL_Texture* gEnemyProjectileTexture    = nullptr;
SDL_Texture* gNinjaProjectileTex        = nullptr;
SDL_Texture* gBossProjectileTex1        = nullptr;
SDL_Texture* gBossProjectileTex2        = nullptr;
SDL_Texture* gMapTexture                = nullptr;
SDL_Texture* gBossTexture               = nullptr;
SDL_Texture* gBossMapTexture            = nullptr;
SDL_Texture* gHeartFullTex              = nullptr;
SDL_Texture* gHeartHalfTex              = nullptr;
SDL_Texture* gHeartEmptyTex             = nullptr;
SDL_Texture* gButtonTex                 = nullptr;
SDL_Texture* gTitleBgTex                = nullptr;
SDL_Texture* gComputerTex               = nullptr;
SDL_Texture* gComputerOnTex             = nullptr;
SDL_Texture* gMazeWallTex               = nullptr;
SDL_Texture* gMazeFloorTex              = nullptr;
SDL_Texture* gMazeArrowTex              = nullptr;
SDL_Texture* gMazePlayerIconTex[5]      = {};
TTF_Font*    gFont                      = nullptr;

// [Internal - map system]
static SDL_Texture* gMapBaseTex             = nullptr;
static SDL_Texture* gDoorTexU               = nullptr;
static SDL_Texture* gDoorTexD               = nullptr;
static SDL_Texture* gDoorTexL               = nullptr;
static SDL_Texture* gDoorTexR               = nullptr;
static SDL_Texture* gDoorOpenTexU           = nullptr;
static SDL_Texture* gDoorOpenTexD           = nullptr;
static SDL_Texture* gDoorOpenTexL           = nullptr;
static SDL_Texture* gDoorOpenTexR           = nullptr;
static SDL_Texture* gRoomMapTextures[16]    = {};
static SDL_Texture* gRoomMapTexturesOpen[16]= {};

// meta PNG parsed data
static int      metaCollision[MAP_ROWS][MAP_COLS] = {};
static SDL_Rect doorRects[4]    = {};  // 0=U 1=D 2=L 3=R  (screen coords)
static bool     collisionReady  = false;
static bool     doorsReady      = false;

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

// collision PNG → out 배열 (magenta=1 wall, yellow=2 obstacle, else=0)
static void LoadCollisionFromPNG(const std::string& path, int out[][MAP_COLS]) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) { std::cout << "Collision not found: " << path << "\n"; return; }
    SDL_Surface* img = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surf);
    if (!img) return;

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
            if      (IsMagenta(r, g, b)) out[row][col] = 1;
            else if (IsYellow (r, g, b)) out[row][col] = 2;
            else                         out[row][col] = 0;
        }
    }
    SDL_UnlockSurface(img);
    SDL_FreeSurface(img);
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


// [Public]
auto LoadAllImages(SDL_Renderer* renderer) -> void
{
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cout << "SDL_image init failed: " << IMG_GetError() << std::endl;
        return;
    }

    auto LoadTexture = [&](const std::string& path) -> SDL_Texture* {
        SDL_Texture* t = IMG_LoadTexture(renderer, path.c_str());
        if (!t) std::cout << "Load failed: " << path << " (" << IMG_GetError() << ")\n";
        return t;
    };

    // stage - base
    gWallTexture            = LoadTexture(AssetPath("map/transparent.png"));
    gObstacleTex            = LoadTexture(AssetPath("map/obstacle.png"));
    gMapTexture             = LoadTexture(AssetPath("map/base/map-base-001.png"));

    // stage - boss
    gBossMapTexture         = LoadTexture(AssetPath("map/boss/map-boss.png"));

    // stage - maze
    gMazeWallTex            = LoadTexture(AssetPath("map/maze/wall.png"));
    gMazeFloorTex           = LoadTexture(AssetPath("map/maze/floor.png"));
    gMazeArrowTex           = LoadTexture(AssetPath("map/maze/ui/arrow.png"));

    for (int i = 0; i < 5; i++)
    {
        std::string path    = "map/maze/ui/character_icon_" + std::to_string(i + 1) + ".png";
        gMazePlayerIconTex[i] = LoadTexture(AssetPath(path.c_str()));
    }

    // player
    gPlayerIdleTex          = LoadTexture(AssetPath("player/body/default/idle.png"));
    gPlayerWalkTex          = LoadTexture(AssetPath("player/body/default/walk.png"));
    gPlayerHurtTex          = LoadTexture(AssetPath("player/body/default/hurt.png"));
    gPlayerDeathTex         = LoadTexture(AssetPath("player/body/default/death.png"));
    gProjectileTexture      = LoadTexture(AssetPath("player/projectile/semi-colon.png"));

    // mob - plain
    gNormalEnemyTex         = LoadTexture(AssetPath("mob/normal.png"));
    gNinjaEnemyTex          = LoadTexture(AssetPath("mob/ninja.png"));
    gSuicideEnemyTex        = LoadTexture(AssetPath("mob/suicide.png"));
    gEnemyProjectileTexture = LoadTexture(AssetPath("mob/projectile/default.png"));
    gNinjaProjectileTex     = LoadTexture(AssetPath("mob/projectile/ninja.png"));

    // mob - boss
    gBossTexture            = LoadTexture(AssetPath("mob/boss.png"));
    gBossProjectileTex1     = LoadTexture(AssetPath("mob/projectile/boss-001.png"));
    gBossProjectileTex2     = LoadTexture(AssetPath("mob/projectile/boss-002.png"));

    // ui - hp
    gHeartFullTex           = LoadTexture(AssetPath("ui/hp/heart_full.png"));
    gHeartHalfTex           = LoadTexture(AssetPath("ui/hp/heart_half.png"));
    gHeartEmptyTex          = LoadTexture(AssetPath("ui/hp/heart_empty.png"));

    // ui - menu
    gButtonTex              = LoadTexture(AssetPath("ui/menu/button.png"));
    gTitleBgTex             = LoadTexture(AssetPath("ui/menu/main.png"));
    gComputerTex            = LoadTexture(AssetPath("map/quiz/computer_off.png"));
    gComputerOnTex          = LoadTexture(AssetPath("map/quiz/computer_on.png"));

    // font
    gFont = TTF_OpenFont(AssetPath("ui/font/Pretendard-Regular.ttf").c_str(), 24);

    if (!gFont)
    {
        std::cout << "Font Load Failed: " << TTF_GetError() << "\n";
    }

    // base map (map/base/map_base_001.png, fallback to map/map_base.png)
    gMapBaseTex = LoadTexture(AssetPath("map/base/map-base-001.png"));
    if (!gMapBaseTex)
    {
        gMapBaseTex = LoadTexture(AssetPath("map/map-base-001.png"));
    }

    // base map collision
    LoadCollisionFromPNG(AssetPath("map/base/map-base-001_collision.png"), metaCollision);
    collisionReady = true;

    // doors PNG
    auto LoadAndParse = [](const std::string& path, void(*parseFn)(SDL_Surface*)) {
        SDL_Surface* surf = IMG_Load(path.c_str());
        if (!surf) { std::cout << "Meta not found: " << path << "\n"; return; }
        SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (rgba) { parseFn(rgba); SDL_FreeSurface(rgba); }
    };
    LoadAndParse(AssetPath("map/base/map-base-001_doors.png"), ParseDoorsPNG);

    // door sprites (closed)
    struct { const char* path; SDL_Texture** tex; } doors[4] = {
        { "map/door/door-up.png",       &gDoorTexU },
        { "map/door/door-down.png",     &gDoorTexD },
        { "map/door/door-left.png",     &gDoorTexL },
        { "map/door/door-right.png",    &gDoorTexR },
    };
    for (auto& d : doors)
        *d.tex = LoadTexture(AssetPath(d.path));

    // door sprites (open — used when room is cleared)
    struct { const char* path; SDL_Texture** tex; } doorsOpen[4] = {
        { "map/door/door-open-up.png",    &gDoorOpenTexU },
        { "map/door/door-open-down.png",  &gDoorOpenTexD },
        { "map/door/door-open-left.png",  &gDoorOpenTexL },
        { "map/door/door-open-right.png", &gDoorOpenTexR },
    };
    for (auto& d : doorsOpen)
        *d.tex = LoadTexture(AssetPath(d.path));
}

void LoadCollisionMapsFromImages()
{
    LoadCollisionFromPNG(AssetPath("map/boss/map-boss_collision.png"), bossMapLayout);
}

// build collision for (doorMask, variant): base or variant PNG + clear active door rects
auto LoadRoomCollisionMap(int doorMask, int variant) -> void
{
    int slot = variant * MAX_PATTERNS + doorMask;
    if (collisionLoaded[slot]) return;

    if (variant == 0) {
        // 기본 콜리전
        if (collisionReady)
            memcpy(mapLayouts[slot], metaCollision, sizeof(metaCollision));
        else
            memset(mapLayouts[slot], 0, sizeof(mapLayouts[0]));
    } else {
        // 변형 콜리전 A / B / C
        char letter = (char)('A' + (variant - 1));
        std::string varPath = std::string("map/base/map-base-001_collision-") + letter + ".png";
        memset(mapLayouts[slot], 0, sizeof(mapLayouts[0]));
        LoadCollisionFromPNG(AssetPath(varPath.c_str()), mapLayouts[slot]);
    }

    for (int d = 0; d < 4; d++)
    {
        if (!(doorMask & (1 << d))) continue;

        SDL_Rect& rect = doorRects[d];
        if (rect.w == 0 || rect.h == 0) continue;

        int colMin = rect.x / TILE_SIZE;
        int colMax = (rect.x + rect.w - 1) / TILE_SIZE;
        int rowMin = rect.y / TILE_SIZE;
        int rowMax = (rect.y + rect.h - 1) / TILE_SIZE;

        for (int row = rowMin; row <= rowMax && row < MAP_ROWS; row++)
            for (int col = colMin; col <= colMax && col < MAP_COLS; col++)
                mapLayouts[slot][row][col] = 0;
    }

    collisionLoaded[slot] = true;
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

// compose base + open door sprites into a cached render-target texture (cleared state)
SDL_Texture* GetRoomMapTextureOpen(SDL_Renderer* renderer, int doorMask) {
    if (gRoomMapTexturesOpen[doorMask]) return gRoomMapTexturesOpen[doorMask];

    SDL_Texture* composed = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!composed) return nullptr;

    SDL_SetTextureBlendMode(composed, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, composed);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (gMapBaseTex) SDL_RenderCopy(renderer, gMapBaseTex, NULL, NULL);

    SDL_Texture* openTexs[4] = { gDoorOpenTexU, gDoorOpenTexD, gDoorOpenTexL, gDoorOpenTexR };
    SDL_Texture* closedTexs[4] = { gDoorTexU, gDoorTexD, gDoorTexL, gDoorTexR };
    for (int d = 0; d < 4; d++) {
        if (!(doorMask & (1 << d))) continue;
        SDL_Rect dst = doorRects[d];
        if (dst.w == 0 || dst.h == 0) continue;
        SDL_Texture* tex = openTexs[d] ? openTexs[d] : closedTexs[d];
        if (!tex) continue;
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }

    SDL_SetRenderTarget(renderer, NULL);
    gRoomMapTexturesOpen[doorMask] = composed;
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
        if (gRoomMapTexturesOpen[i]) SDL_DestroyTexture(gRoomMapTexturesOpen[i]);
        gRoomMapTexturesOpen[i] = nullptr;
    }

    auto Free = [](SDL_Texture*& t) { if (t) { SDL_DestroyTexture(t); t = nullptr; } };
    Free(gPlayerIdleTex);
    Free(gPlayerWalkTex);
    Free(gPlayerHurtTex);
    Free(gPlayerDeathTex);
    Free(gWallTexture);
    Free(gFloorTexture);
    Free(gObstacleTex);
    Free(gNormalEnemyTex);
    Free(gNinjaEnemyTex);
    Free(gSuicideEnemyTex);
    Free(gProjectileTexture);
    Free(gEnemyProjectileTexture);
    Free(gNinjaProjectileTex);
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
    Free(gComputerTex);
    Free(gComputerOnTex);
    Free(gMazeWallTex);
    Free(gMazeFloorTex);
    Free(gMazeArrowTex);
    for (int i = 0; i < 5; i++) Free(gMazePlayerIconTex[i]);
    Free(gMapBaseTex);
    Free(gDoorTexU);
    Free(gDoorTexD);
    Free(gDoorTexL);
    Free(gDoorTexR);
    Free(gDoorOpenTexU);
    Free(gDoorOpenTexD);
    Free(gDoorOpenTexL);
    Free(gDoorOpenTexR);

    IMG_Quit();
}
