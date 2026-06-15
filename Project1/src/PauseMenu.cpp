#pragma execution_character_set("utf-8")
#include "PauseMenu.hpp"
#include "TextRenderer.hpp"
#include "ImageManager.hpp"
#include "Item.hpp"
#include "Constants.h"

// 버튼 위치 — 보드(y 15~215) 아래
SDL_Rect resumeBtn = { 300, 340, 200, 50 };
SDL_Rect helpBtn   = { 300, 410, 200, 50 };
SDL_Rect quitBtn   = { 300, 480, 200, 50 };

// 아이템 보드 레이아웃 상수
static const int BOARD_X       = 150;
static const int BOARD_Y       = 45;
static const int BOARD_W       = 500;
static const int BOARD_H       = 200;
static const int ICON_SIZE     = 40;
static const int ICON_GAP      = 10;
static const int ICON_STEP     = ICON_SIZE + ICON_GAP;  // 50
static const int ICONS_PER_ROW = 8;
static const int ICON_X0       = BOARD_X + 40;
static const int ICON_Y0       = BOARD_Y + 45;

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

// 아이콘 rect 계산 (인벤토리 인덱스 → 화면 좌표)
static SDL_Rect IconRect(int idx) {
    int col = idx % ICONS_PER_ROW;
    int row = idx / ICONS_PER_ROW;
    return { ICON_X0 + col * ICON_STEP, ICON_Y0 + row * ICON_STEP, ICON_SIZE, ICON_SIZE };
}

void DrawPauseMenu(SDL_Renderer* renderer, const PlayerData* player)
{
    // 반투명 어두운 오버레이
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect full = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &full);

    // ── 아이템 보드 ──
    SDL_Rect boardRect = { BOARD_X, BOARD_Y, BOARD_W, BOARD_H };
    if (gPauseBoardTex) {
        SDL_RenderCopy(renderer, gPauseBoardTex, NULL, &boardRect);
    } else {
        // 에셋 없을 때 폴백 박스
        SDL_SetRenderDrawColor(renderer, 40, 40, 60, 220);
        SDL_RenderFillRect(renderer, &boardRect);
        SDL_SetRenderDrawColor(renderer, 160, 160, 200, 255);
        SDL_RenderDrawRect(renderer, &boardRect);
        SDL_Color gray = { 180, 180, 180, 255 };
        DrawText(renderer, "Items", BOARD_X + 10, BOARD_Y + 10, gray);
    }

    // ── 아이템 아이콘 목록 ──
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int hoveredIdx = -1;

    for (int i = 0; i < player->itemCount; i++) {
        int type = player->collectedItems[i];
        SDL_Rect ir = IconRect(i);

        // 아이콘 렌더
        if (type >= 0 && type < ITEM_COUNT && gItemTextures[type]) {
            SDL_RenderCopy(renderer, gItemTextures[type], NULL, &ir);
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
            SDL_RenderFillRect(renderer, &ir);
        }

        // 호버 하이라이트
        bool hovered = (mx >= ir.x && mx < ir.x + ir.w &&
                        my >= ir.y && my < ir.y + ir.h);
        if (hovered) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
            SDL_RenderFillRect(renderer, &ir);
            hoveredIdx = i;
        }

        // 아이콘 테두리
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 180);
        SDL_RenderDrawRect(renderer, &ir);
    }

    // ── 툴팁 ──
    if (hoveredIdx >= 0) {
        int type = player->collectedItems[hoveredIdx];
        const char* name = GetItemName(type);
        const char* desc = GetItemDescription(type);

        const int TW = 220, TH = 62;
        SDL_Rect ir   = IconRect(hoveredIdx);
        int tx = ir.x + ir.w / 2 - TW / 2;
        int ty = ir.y - TH - 6;
        // 화면 밖으로 나가면 아래쪽에 표시
        if (ty < 0)              ty = ir.y + ir.h + 6;
        if (tx < 0)              tx = 0;
        if (tx + TW > SCREEN_WIDTH) tx = SCREEN_WIDTH - TW;

        SDL_Rect tipBox = { tx, ty, TW, TH };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 220);
        SDL_RenderFillRect(renderer, &tipBox);
        SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
        SDL_RenderDrawRect(renderer, &tipBox);

        SDL_Color white  = { 255, 255, 255, 255 };
        SDL_Color yellow = { 255, 230,  80, 255 };
        DrawText(renderer, name, tx + 8, ty + 8,  white);
        DrawText(renderer, desc, tx + 8, ty + 34, yellow);
    }

    // ── 버튼 ──
    DrawButton(renderer, resumeBtn);
    DrawButton(renderer, helpBtn);
    DrawButton(renderer, quitBtn);

    DrawTextCenter(renderer, "계속하기", resumeBtn.y + 12);
    DrawTextCenter(renderer, "조작키",   helpBtn.y   + 12);
    DrawTextCenter(renderer, "나가기",   quitBtn.y   + 12);
}

bool PauseResumeClicked(int x, int y)
{
    return (x >= resumeBtn.x && x <= resumeBtn.x + resumeBtn.w &&
            y >= resumeBtn.y && y <= resumeBtn.y + resumeBtn.h);
}

bool PauseHelpClicked(int x, int y)
{
    return (x >= helpBtn.x && x <= helpBtn.x + helpBtn.w &&
            y >= helpBtn.y && y <= helpBtn.y + helpBtn.h);
}

bool PauseQuitClicked(int x, int y)
{
    return (x >= quitBtn.x && x <= quitBtn.x + quitBtn.w &&
            y >= quitBtn.y && y <= quitBtn.y + quitBtn.h);
}
