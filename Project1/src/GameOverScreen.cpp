#pragma execution_character_set("utf-8")
#include "GameOverScreen.hpp"
#include "ImageManager.hpp"
#include "TextRenderer.hpp"
#include "Constants.h"

static const SDL_Rect RESTART_BTN  = { 300, 400, 200, 50 };
static const SDL_Rect LOGO_RECT    = { SCREEN_WIDTH / 2 - 200, 150, 400, 160 };

void DrawGameOverScreen(SDL_Renderer* renderer)
{
    SDL_Rect full = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // 배경
    if (gGameOverTex) {
        SDL_RenderCopy(renderer, gGameOverTex, NULL, &full);
    } else {
        SDL_SetRenderDrawColor(renderer, 12, 5, 5, 255);
        SDL_RenderFillRect(renderer, &full);
    }

    // 게임오버 로고 (버튼 위 중앙)
    if (gGameOverLogoTex) {
        SDL_SetTextureBlendMode(gGameOverLogoTex, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, gGameOverLogoTex, NULL, &LOGO_RECT);
    } else {
        SDL_Color red = { 210, 40, 40, 255 };
        DrawTextCenter(renderer, "GAME OVER", 200, red);
    }

    // 다시하기 버튼
    SDL_Color white = { 255, 255, 255, 255 };

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    bool hov = mx >= RESTART_BTN.x && mx < RESTART_BTN.x + RESTART_BTN.w &&
               my >= RESTART_BTN.y && my < RESTART_BTN.y + RESTART_BTN.h;

    if (gButtonTex) {
        SDL_RenderCopy(renderer, gButtonTex, NULL, &RESTART_BTN);
        if (hov) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
            SDL_RenderFillRect(renderer, &RESTART_BTN);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, hov ? 130 : 80, hov ? 130 : 80, hov ? 130 : 80, 255);
        SDL_RenderFillRect(renderer, &RESTART_BTN);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &RESTART_BTN);
    }

    DrawTextInRect(renderer, "다시하기", RESTART_BTN, white);
}

bool GameOverRestartClicked(int x, int y)
{
    return x >= RESTART_BTN.x && x <= RESTART_BTN.x + RESTART_BTN.w &&
           y >= RESTART_BTN.y && y <= RESTART_BTN.y + RESTART_BTN.h;
}
