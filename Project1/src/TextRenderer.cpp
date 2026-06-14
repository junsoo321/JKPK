#pragma execution_character_set("utf-8")
#include "TextRenderer.hpp"
#include "Constants.h"
#include <SDL_ttf.h>

extern TTF_Font* gFont;

//TTF 폰트 블렌딩 서피스 생성 후 텍스처 변환 및 메모리 해제(파괴) 흐름 제어
void DrawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color)
{
    if (!gFont) return;

    // UTF-8 문자열을 고품질(안티에일리어싱 블렌디드) 서피스로 생성
    SDL_Surface* surface = TTF_RenderUTF8_Blended(gFont, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { x, y, surface->w, surface->h };

    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    // [중요] 매 프레임 호출되는 드로우 함수이므로 메모리 누수(Leak) 방지를 위해 즉시 해제
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

//출력할 텍스트의 실제 픽셀 가로 길이를 미리 측정하여 전체 해상도 기준 중앙 X 좌표 산출
void DrawTextCenter(SDL_Renderer* renderer, const std::string& text, int y, SDL_Color color)
{
    if (!gFont) return;

    int textW = 0;
    int textH = 0;

    // 렌더링을 직접 해보지 않고도 텍스트가 차지할 픽셀 폭(textW)을 알아내는 함수
    TTF_SizeUTF8(gFont, text.c_str(), &textW, &textH);

    int x = (SCREEN_WIDTH - textW) / 2;

    DrawText(renderer, text, x, y, color);
}
