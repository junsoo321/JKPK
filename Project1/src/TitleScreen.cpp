#pragma execution_character_set("utf-8")
#include "TitleScreen.hpp"
#include "Constants.h"
#include "TextRenderer.hpp"
#include "ImageManager.hpp"
#include <SDL.h>

SDL_Rect startBtn = { 300, 300, 200, 60 };
SDL_Rect exitBtn  = { 300, 380, 200, 60 };

static void DrawButton(SDL_Renderer* renderer, SDL_Rect rect)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    bool hovered = (mx >= rect.x && mx < rect.x + rect.w &&
                    my >= rect.y && my < rect.y + rect.h);

    if (gButtonTex) {
        SDL_RenderCopy(renderer, gButtonTex, NULL, &rect);
        if (hovered) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
            SDL_RenderFillRect(renderer, &rect);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, hovered ? 120 : 80, hovered ? 120 : 80, hovered ? 120 : 80, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void DrawTitleScreen(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
    SDL_RenderClear(renderer);

    DrawTextCenter(renderer, "폐 연구실 탈출 게임", 150);
    DrawTextCenter(renderer, "JKPK팀", 220);

    DrawButton(renderer, startBtn);
    DrawButton(renderer, exitBtn);

    DrawText(renderer, "게임시작", startBtn.x + 55, startBtn.y + 15);
    DrawText(renderer, "종료",     exitBtn.x  + 75, exitBtn.y  + 15);
}
bool TitleStartClicked(int x, int y){
    return x >= startBtn.x &&
        x <= startBtn.x + startBtn.w &&
        y >= startBtn.y &&
        y <= startBtn.y + startBtn.h;
}

bool TitleExitClicked(int x, int y){
    return x >= exitBtn.x &&
        x <= exitBtn.x + exitBtn.w &&
        y >= exitBtn.y &&
        y <= exitBtn.y + exitBtn.h;
}
