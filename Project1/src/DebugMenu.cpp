#ifdef _DEBUG

#include "DebugMenu.hpp"
#include "Constants.h"
#include "TextRenderer.hpp"
#include "GameState.hpp"
#include <SDL.h>
#include <string>

// --- Password state ---
static const char* DEBUG_PASSWORD = "1234";
static const int   MAX_INPUT_LEN  = 4;

static bool        gOpen     = false;
static bool        gUnlocked = false;
static std::string gInput    = "";
static bool        gWrong    = false;
static Uint32      gWrongAt  = 0;

// --- Menu state ---
static bool   gMenuOpen     = false;
static int    gDepth        = 0;  // 0=root, 1=map submenu, 2=mob submenu
static int    gRootIdx      = 0;
static int    gMapIdx       = 0;
static int    gMobIdx       = 0;
static Uint32 gMobWarnUntil = 0;

static DebugAction gPendingAction = DEBUG_ACTION_NONE;

static const char* ROOT_ITEMS[] = { "Map", "Mob" };
static const int   ROOT_COUNT   = 2;

static const char* MAP_ITEMS[]  = { "Boss", "Maze", "Quiz", "Plain" };
static const int   MAP_COUNT    = 4;

static const char* MOB_ITEMS[]  = { "Normal", "Ninja", "Suicide" };
static const int   MOB_COUNT    = 3;

// --- Password helpers ---
static void OpenPassword() {
    gOpen    = true;
    gInput   = "";
    gWrong   = false;
    gWrongAt = 0;
    SDL_StartTextInput();
}

static void ClosePassword() {
    gOpen  = false;
    gInput = "";
    gWrong = false;
    SDL_StopTextInput();
}

static void Confirm() {
    if (gInput == DEBUG_PASSWORD) {
        gUnlocked = true;
        gMenuOpen = true;
        gDepth    = 0;
        gRootIdx  = 0;
        ClosePassword();
    } else {
        gInput   = "";
        gWrong   = true;
        gWrongAt = SDL_GetTicks();
    }
}

// --- Public API ---
void DebugMenuHandleEvent(SDL_Event& event) {
    // Shift+T: open password prompt (or re-open menu if already unlocked)
    if (!gOpen && !gMenuOpen && event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_t &&
            (event.key.keysym.mod & KMOD_SHIFT)) {
            if (gUnlocked) {
                gMenuOpen = true;
                gDepth    = 0;
                gRootIdx  = 0;
            } else {
                OpenPassword();
            }
            return;
        }
    }

    // Password prompt input
    if (gOpen) {
        if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;
            if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                Confirm();
            } else if (key == SDLK_ESCAPE) {
                ClosePassword();
            } else if (key == SDLK_BACKSPACE && !gInput.empty()) {
                gInput.pop_back();
                gWrong = false;
            }
        }
        if (event.type == SDL_TEXTINPUT) {
            if ((int)gInput.size() < MAX_INPUT_LEN) {
                gInput += event.text.text;
                gWrong  = false;
            }
        }
        return;
    }

    // Menu navigation
    if (gMenuOpen && event.type == SDL_KEYDOWN) {
        SDL_Keycode key = event.key.keysym.sym;
        if (gDepth == 0) {
            if      (key == SDLK_UP)    gRootIdx = (gRootIdx - 1 + ROOT_COUNT) % ROOT_COUNT;
            else if (key == SDLK_DOWN)  gRootIdx = (gRootIdx + 1) % ROOT_COUNT;
            else if (key == SDLK_RIGHT) {
                if      (gRootIdx == 0) { gDepth = 1; gMapIdx = 0; }
                else if (gRootIdx == 1) { gDepth = 2; gMobIdx = 0; gMobWarnUntil = 0; }
            }
            else if (key == SDLK_LEFT || key == SDLK_ESCAPE) { gMenuOpen = false; }
        } else if (gDepth == 1) {
            if      (key == SDLK_UP)   gMapIdx = (gMapIdx - 1 + MAP_COUNT) % MAP_COUNT;
            else if (key == SDLK_DOWN) gMapIdx = (gMapIdx + 1) % MAP_COUNT;
            else if (key == SDLK_RIGHT) {
                static const DebugAction ACTIONS[4] = {
                    DEBUG_ACTION_MAP_BOSS, DEBUG_ACTION_MAP_MAZE,
                    DEBUG_ACTION_MAP_QUIZ, DEBUG_ACTION_MAP_PLAIN
                };
                gPendingAction = ACTIONS[gMapIdx];
                gMenuOpen      = false;
            }
            else if (key == SDLK_LEFT)   { gDepth = 0; }
            else if (key == SDLK_ESCAPE) { gMenuOpen = false; }
        } else if (gDepth == 2) {
            if      (key == SDLK_UP)   gMobIdx = (gMobIdx - 1 + MOB_COUNT) % MOB_COUNT;
            else if (key == SDLK_DOWN) gMobIdx = (gMobIdx + 1) % MOB_COUNT;
            else if (key == SDLK_RIGHT) {
                if (gGameState != GAME_NORMAL) {
                    gMobWarnUntil = SDL_GetTicks() + 2000;
                } else {
                    static const DebugAction SPAWN_ACTIONS[3] = {
                        DEBUG_ACTION_SPAWN_NORMAL, DEBUG_ACTION_SPAWN_NINJA, DEBUG_ACTION_SPAWN_SUICIDE
                    };
                    gPendingAction = SPAWN_ACTIONS[gMobIdx];
                    gMenuOpen      = false;
                }
            }
            else if (key == SDLK_LEFT)   { gDepth = 0; }
            else if (key == SDLK_ESCAPE) { gMenuOpen = false; }
        }
    }
}

void DebugMenuRender(SDL_Renderer* renderer) {
    // -- Password prompt --
    if (gOpen) {
        if (gWrong && SDL_GetTicks() - gWrongAt > 1000) gWrong = false;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, nullptr);

        const int BOX_W = 320, BOX_H = 160;
        SDL_Rect box = {
            SCREEN_WIDTH  / 2 - BOX_W / 2,
            SCREEN_HEIGHT / 2 - BOX_H / 2,
            BOX_W, BOX_H
        };
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &box);

        SDL_Color white  = { 255, 255, 255, 255 };
        SDL_Color yellow = { 255, 220,  50, 255 };
        SDL_Color red    = { 255,  80,  80, 255 };

        DrawTextCenter(renderer, "[DEBUG] Password", box.y + 20, white);

        std::string stars(gInput.size(), '*');
        while ((int)stars.size() < MAX_INPUT_LEN) stars += '_';
        DrawTextCenter(renderer, stars, box.y + 65, yellow);

        DrawTextCenter(renderer,
            gWrong ? "Wrong password" : "Enter: OK   ESC: Cancel",
            box.y + 110, gWrong ? red : white);
        return;
    }

    // -- Navigation menu --
    if (!gMenuOpen) return;

    const char** items  = (gDepth == 0) ? ROOT_ITEMS : (gDepth == 1) ? MAP_ITEMS : MOB_ITEMS;
    int          count  = (gDepth == 0) ? ROOT_COUNT  : (gDepth == 1) ? MAP_COUNT : MOB_COUNT;
    int          selIdx = (gDepth == 0) ? gRootIdx    : (gDepth == 1) ? gMapIdx   : gMobIdx;

    bool showWarn = (gDepth == 2) && (gGameState != GAME_NORMAL);
    bool showTimedWarn = (gDepth == 2) && (SDL_GetTicks() < gMobWarnUntil);

    const int ITEM_H  = 32;
    const int BOX_W   = 280;
    const int WARN_H  = showWarn ? 28 : 0;
    const int BOX_H   = 52 + count * ITEM_H + 8 + WARN_H;

    SDL_Rect box = {
        SCREEN_WIDTH  / 2 - BOX_W / 2,
        SCREEN_HEIGHT / 2 - BOX_H / 2,
        BOX_W, BOX_H
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_RenderFillRect(renderer, nullptr);

    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 140, 140, 255, 255);
    SDL_RenderDrawRect(renderer, &box);

    SDL_Color white  = { 255, 255, 255, 255 };
    SDL_Color yellow = { 255, 220,  50, 255 };
    SDL_Color gray   = { 160, 160, 160, 255 };
    SDL_Color red    = { 255,  60,  60, 255 };

    const char* title = (gDepth == 0) ? "[DEBUG MENU]"
                      : (gDepth == 1) ? "[DEBUG] > Map"
                      :                 "[DEBUG] > Mob";
    DrawTextCenter(renderer, title, box.y + 12, white);

    for (int i = 0; i < count; i++) {
        bool sel = (i == selIdx);
        std::string label = sel ? "> " : "  ";
        label += items[i];
        DrawTextCenter(renderer, label, box.y + 48 + i * ITEM_H, sel ? yellow : white);
    }

    if (showWarn) {
        const char* warnMsg = showTimedWarn
            ? "! 일반 맵에서만 사용 가능 !"
            : "[ 일반 맵에서만 사용 가능 ]";
        DrawTextCenter(renderer, warnMsg, box.y + 48 + count * ITEM_H + 4, red);
    }

    DrawTextCenter(renderer,
        "UP/DOWN: Select   RIGHT: Enter   LEFT: Back",
        box.y + BOX_H + 8, gray);
}

bool        DebugMenuIsOpen()     { return gOpen;     }
bool        DebugMenuIsMenuOpen() { return gMenuOpen;  }
bool        DebugMenuIsUnlocked() { return gUnlocked;  }
DebugAction DebugMenuGetAction()  {
    DebugAction act = gPendingAction;
    gPendingAction  = DEBUG_ACTION_NONE;
    return act;
}

#endif
