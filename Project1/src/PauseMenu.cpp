#pragma execution_character_set("utf-8")
#include "PauseMenu.hpp"
#include "TextRenderer.hpp"
#include "ImageManager.hpp"
#include "Constants.h"

SDL_Rect resumeBtn = { 300, 220, 200, 50 };
SDL_Rect helpBtn   = { 300, 290, 200, 50 };
SDL_Rect quitBtn   = { 300, 360, 200, 50 };

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
        SDL_SetRenderDrawColor(renderer, hovered ? 130 : 90, hovered ? 130 : 90, hovered ? 130 : 90, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void DrawPauseMenu(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect full = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &full);

    DrawButton(renderer, resumeBtn);
    DrawButton(renderer, helpBtn);
    DrawButton(renderer, quitBtn);

    DrawTextCenter(renderer, "계속하기", resumeBtn.y + 12);
    DrawTextCenter(renderer, "조작키",   helpBtn.y   + 12);
    DrawTextCenter(renderer, "나가기",   quitBtn.y   + 12);
}

// 입력받은 마우스 좌표(x, y)가 '계속하기' 버튼 영역 내부에 있는지 검증
bool PauseResumeClicked(int x, int y)
{
    return (x >= resumeBtn.x && x <= resumeBtn.x + resumeBtn.w &&
        y >= resumeBtn.y && y <= resumeBtn.y + resumeBtn.h);
}

// 입력받은 마우스 좌표(x, y)가 '조작키' 버튼 영역 내부에 있는지 검증
bool PauseHelpClicked(int x, int y)
{
    return (x >= helpBtn.x && x <= helpBtn.x + helpBtn.w &&
        y >= helpBtn.y && y <= helpBtn.y + helpBtn.h);
}

// 입력받은 마우스 좌표(x, y)가 '나가기' 버튼 영역 내부에 있는지 검증
bool PauseQuitClicked(int x, int y)
{
    return (x >= quitBtn.x && x <= quitBtn.x + quitBtn.w &&
        y >= quitBtn.y && y <= quitBtn.y + quitBtn.h);
}
