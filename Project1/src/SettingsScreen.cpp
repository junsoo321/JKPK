#pragma execution_character_set("utf-8")
#include "SettingsScreen.hpp"
#include "ImageManager.hpp"
#include "TextRenderer.hpp"
#include "Constants.h"
#include <windows.h>
#undef DrawText   // windows.h가 DrawText를 DrawTextW 매크로로 재정의하는 것 방지
#include <string>
#include <vector>
#include <algorithm>

static std::vector<std::string> gFontNames;
static int gScrollOffset = 0;

static const int LIST_X    = 200;
static const int LIST_Y    = 135;
static const int LIST_W    = 400;
static const int ITEM_H    = 40;
static const int VISIBLE   = 7;

static SDL_Rect backBtn = { 300, 500, 200, 50 };

static void ScanFonts() {
    gFontNames.clear();
    std::string dir = GetFontDirPath();
    std::string pattern = dir + "*.ttf";
    for (char& c : pattern) if (c == '/') c = '\\';

    // UTF-8 경로 → Wide 변환 후 FindFirstFileW 사용 (한글 파일명 지원)
    int wlen = MultiByteToWideChar(CP_UTF8, 0, pattern.c_str(), -1, nullptr, 0);
    std::wstring wpattern(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, pattern.c_str(), -1, &wpattern[0], wlen);

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(wpattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            int len = WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, nullptr, 0, nullptr, nullptr);
            std::string utf8name(len - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, &utf8name[0], len, nullptr, nullptr);
            gFontNames.push_back(utf8name);
        }
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);

    std::sort(gFontNames.begin(), gFontNames.end());
}

void InitSettingsScreen() {
    ScanFonts();
    gScrollOffset = 0;

    // 현재 선택된 폰트가 보이도록 스크롤 이동
    const char* cur = GetCurrentFontName();
    for (int i = 0; i < (int)gFontNames.size(); i++) {
        if (gFontNames[i] == cur) {
            int ideal = i - VISIBLE / 2;
            int maxScroll = (int)gFontNames.size() - VISIBLE;
            gScrollOffset = ideal < 0 ? 0 : (ideal > maxScroll ? maxScroll : ideal);
            break;
        }
    }
}

void DrawSettingsScreen(SDL_Renderer* renderer) {
    SDL_Color white  = { 255, 255, 255, 255 };
    SDL_Color yellow = { 255, 230,  80, 255 };
    SDL_Color gray   = { 160, 160, 160, 255 };
    SDL_Color green  = {  80, 220, 120, 255 };

    // 배경
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 10, 18, 248);
    SDL_Rect full = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &full);

    DrawTextCenter(renderer, "환경설정",  40, white);
    DrawTextCenter(renderer, "서체 선택", 95, gray);

    if (gFontNames.empty()) {
        DrawTextCenter(renderer, "폰트 파일을 찾을 수 없습니다.", 260, gray);
    } else {
        // 리스트 배경
        int listH = VISIBLE * ITEM_H;
        SDL_Rect listBg = { LIST_X - 4, LIST_Y - 4, LIST_W + 8, listH + 8 };
        SDL_SetRenderDrawColor(renderer, 28, 28, 40, 255);
        SDL_RenderFillRect(renderer, &listBg);
        SDL_SetRenderDrawColor(renderer, 70, 70, 95, 255);
        SDL_RenderDrawRect(renderer, &listBg);

        int mx, my;
        SDL_GetMouseState(&mx, &my);
        const char* curFont = GetCurrentFontName();

        int count = (int)gFontNames.size();
        int end   = gScrollOffset + VISIBLE;
        if (end > count) end = count;

        for (int i = gScrollOffset; i < end; i++) {
            int row = i - gScrollOffset;
            SDL_Rect ir = { LIST_X, LIST_Y + row * ITEM_H, LIST_W, ITEM_H - 1 };

            bool isCurrent = (gFontNames[i] == curFont);
            bool hovered   = mx >= ir.x && mx < ir.x + ir.w &&
                             my >= ir.y && my < ir.y + ir.h;

            if (isCurrent) {
                SDL_SetRenderDrawColor(renderer, 35, 75, 45, 255);
                SDL_RenderFillRect(renderer, &ir);
                SDL_SetRenderDrawColor(renderer, 70, 190, 90, 255);
                SDL_RenderDrawRect(renderer, &ir);
            } else if (hovered) {
                SDL_SetRenderDrawColor(renderer, 45, 45, 68, 255);
                SDL_RenderFillRect(renderer, &ir);
            }

            SDL_Color col = isCurrent ? green : (hovered ? white : gray);
            DrawText(renderer, gFontNames[i], LIST_X + 12, LIST_Y + row * ITEM_H + 10, col);
        }

        // 스크롤 위치 표시
        if (count > VISIBLE) {
            std::string info = std::to_string(gScrollOffset + 1) + " - " +
                               std::to_string(end) + " / " + std::to_string(count);
            DrawTextCenter(renderer, info, LIST_Y + listH + 12, gray);
        }
    }

    // 뒤로 버튼
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    bool backHov = mx >= backBtn.x && mx < backBtn.x + backBtn.w &&
                   my >= backBtn.y && my < backBtn.y + backBtn.h;

    if (gButtonTex) {
        SDL_RenderCopy(renderer, gButtonTex, NULL, &backBtn);
        if (backHov) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
            SDL_RenderFillRect(renderer, &backBtn);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, backHov ? 130 : 90, backHov ? 130 : 90, backHov ? 130 : 90, 255);
        SDL_RenderFillRect(renderer, &backBtn);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &backBtn);
    }
    DrawTextCenter(renderer, "뒤로", backBtn.y + 12, white);
}

void HandleSettingsEvent(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mx = event.button.x, my = event.button.y;
        int count = (int)gFontNames.size();
        int end   = gScrollOffset + VISIBLE;
        if (end > count) end = count;

        for (int i = gScrollOffset; i < end; i++) {
            int row = i - gScrollOffset;
            SDL_Rect ir = { LIST_X, LIST_Y + row * ITEM_H, LIST_W, ITEM_H - 1 };
            if (mx >= ir.x && mx < ir.x + ir.w && my >= ir.y && my < ir.y + ir.h) {
                ReloadFont(gFontNames[i].c_str());
                return;
            }
        }
    }

    if (event.type == SDL_MOUSEWHEEL) {
        gScrollOffset -= event.wheel.y;
        int maxScroll = (int)gFontNames.size() - VISIBLE;
        if (maxScroll < 0) maxScroll = 0;
        if (gScrollOffset < 0) gScrollOffset = 0;
        if (gScrollOffset > maxScroll) gScrollOffset = maxScroll;
    }

    if (event.type == SDL_KEYDOWN) {
        int maxScroll = (int)gFontNames.size() - VISIBLE;
        if (maxScroll < 0) maxScroll = 0;
        if (event.key.keysym.sym == SDLK_UP   && gScrollOffset > 0)         gScrollOffset--;
        if (event.key.keysym.sym == SDLK_DOWN  && gScrollOffset < maxScroll) gScrollOffset++;
    }
}

bool SettingsBackClicked(int x, int y) {
    return x >= backBtn.x && x <= backBtn.x + backBtn.w &&
           y >= backBtn.y && y <= backBtn.y + backBtn.h;
}
