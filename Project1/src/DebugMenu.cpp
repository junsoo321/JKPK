#ifdef _DEBUG

#include "DebugMenu.hpp"
#include "Constants.h"
#include "TextRenderer.hpp"
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
static bool gMenuOpen = false;
static int  gDepth    = 0;  // 0=root, 1=map submenu
static int  gRootIdx  = 0;
static int  gMapIdx   = 0;

static DebugAction gPendingAction = DEBUG_ACTION_NONE;

static const char* ROOT_ITEMS[] = { "Map" };
static const int   ROOT_COUNT   = 1;

static const char* MAP_ITEMS[]  = { "Boss", "Maze", "Quiz", "Plain" };
static const int   MAP_COUNT    = 4;

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
            else if (key == SDLK_RIGHT) { if (gRootIdx == 0) { gDepth = 1; gMapIdx = 0; } }
            else if (key == SDLK_LEFT || key == SDLK_ESCAPE) { gMenuOpen = false; }
        } else {
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

    const char** items  = (gDepth == 0) ? ROOT_ITEMS : MAP_ITEMS;
    int          count  = (gDepth == 0) ? ROOT_COUNT  : MAP_COUNT;
    int          selIdx = (gDepth == 0) ? gRootIdx    : gMapIdx;

    const int ITEM_H = 32;
    const int BOX_W  = 280;
    const int BOX_H  = 52 + count * ITEM_H + 8;

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

    const char* title = (gDepth == 0) ? "[DEBUG MENU]" : "[DEBUG] > Map";
    DrawTextCenter(renderer, title, box.y + 12, white);

    for (int i = 0; i < count; i++) {
        bool sel = (i == selIdx);
        std::string label = sel ? "> " : "  ";
        label += items[i];
        DrawTextCenter(renderer, label, box.y + 48 + i * ITEM_H, sel ? yellow : white);
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
