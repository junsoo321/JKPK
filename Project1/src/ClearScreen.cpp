#pragma execution_character_set("utf-8")
#include "ClearScreen.hpp"
#include "ImageManager.hpp"
#include "TextRenderer.hpp"
#include "Constants.h"

static const SDL_Rect EXIT_BTN  = { 300, 420, 200, 50 };
static const SDL_Rect LOGO_RECT = { SCREEN_WIDTH / 2 - 200, 150, 400, 160 };

void DrawClearScreen(SDL_Renderer* renderer)
{
    SDL_Rect full = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // 배경
    if (gClearTex) {
        SDL_RenderCopy(renderer, gClearTex, NULL, &full);
    } else {
        SDL_SetRenderDrawColor(renderer, 5, 12, 5, 255);
        SDL_RenderFillRect(renderer, &full);
    }

    // 클리어 로고
    if (gClearLogoTex) {
        SDL_SetTextureBlendMode(gClearLogoTex, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, gClearLogoTex, NULL, &LOGO_RECT);
    } else {
        SDL_Color gold = { 255, 210, 50, 255 };
        DrawTextCenter(renderer, "CLEAR!", 200, gold);
    }

    // 종료 버튼
    SDL_Color white = { 255, 255, 255, 255 };

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    bool hov = mx >= EXIT_BTN.x && mx < EXIT_BTN.x + EXIT_BTN.w &&
               my >= EXIT_BTN.y && my < EXIT_BTN.y + EXIT_BTN.h;

    if (gButtonTex) {
        SDL_RenderCopy(renderer, gButtonTex, NULL, &EXIT_BTN);
        if (hov) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
            SDL_RenderFillRect(renderer, &EXIT_BTN);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, hov ? 130 : 80, hov ? 130 : 80, hov ? 130 : 80, 255);
        SDL_RenderFillRect(renderer, &EXIT_BTN);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &EXIT_BTN);
    }

    DrawTextInRect(renderer, "종료", EXIT_BTN, white);
}

bool ClearExitClicked(int x, int y)
{
    return x >= EXIT_BTN.x && x <= EXIT_BTN.x + EXIT_BTN.w &&
           y >= EXIT_BTN.y && y <= EXIT_BTN.y + EXIT_BTN.h;
}
