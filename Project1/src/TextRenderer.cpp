#pragma execution_character_set("utf-8")
#include "TextRenderer.hpp"
#include "Constants.h"
#include <SDL_ttf.h>
#include <vector>

extern TTF_Font* gFont;
extern TTF_Font* gFallbackFont;
extern bool      gFontHasKorean;

// UTF-8 코드포인트 하나를 읽어 (코드포인트, 바이트 길이) 반환
static std::pair<uint32_t, int> ReadCodepoint(const std::string& text, size_t i)
{
    unsigned char b = (unsigned char)text[i];
    if (b < 0x80)  return { b, 1 };
    if ((b & 0xE0) == 0xC0 && i + 1 < text.size())
        return { ((b & 0x1F) << 6) | ((unsigned char)text[i+1] & 0x3F), 2 };
    if ((b & 0xF0) == 0xE0 && i + 2 < text.size()) {
        uint32_t cp = ((b & 0x0F) << 12)
                    | (((unsigned char)text[i+1] & 0x3F) << 6)
                    |  ((unsigned char)text[i+2] & 0x3F);
        return { cp, 3 };
    }
    if ((b & 0xF8) == 0xF0 && i + 3 < text.size()) {
        uint32_t cp = ((b & 0x07) << 18)
                    | (((unsigned char)text[i+1] & 0x3F) << 12)
                    | (((unsigned char)text[i+2] & 0x3F) <<  6)
                    |  ((unsigned char)text[i+3] & 0x3F);
        return { cp, 4 };
    }
    return { b, 1 }; // fallback: treat as single byte
}

static bool IsKoreanSyllable(uint32_t cp)
{
    return cp >= 0xAC00 && cp <= 0xD7A3;
}

// 문자열을 (텍스트, 폰트) 세그먼트로 분리
// 한글 미지원 폰트 사용 시 한글 세그먼트는 gFallbackFont로 렌더링
struct TextSeg { std::string text; TTF_Font* font; };

static std::vector<TextSeg> SplitSegments(const std::string& text)
{
    std::vector<TextSeg> segs;
    if (text.empty()) return segs;

    std::string cur;
    bool curKorean = false;
    bool first = true;

    for (size_t i = 0; i < text.size(); ) {
        std::pair<uint32_t,int> cpi = ReadCodepoint(text, i);
        uint32_t cp = cpi.first;
        int len     = cpi.second;
        bool korean = IsKoreanSyllable(cp);

        if (first || korean != curKorean) {
            if (!first && !cur.empty()) {
                TTF_Font* f = (curKorean && !gFontHasKorean && gFallbackFont)
                              ? gFallbackFont : gFont;
                segs.push_back({ cur, f });
            }
            cur = text.substr(i, len);
            curKorean = korean;
            first = false;
        } else {
            cur += text.substr(i, len);
        }
        i += len;
    }

    if (!cur.empty()) {
        TTF_Font* f = (curKorean && !gFontHasKorean && gFallbackFont)
                      ? gFallbackFont : gFont;
        segs.push_back({ cur, f });
    }

    return segs;
}

static int TotalWidth(const std::vector<TextSeg>& segs)
{
    int total = 0;
    for (auto& s : segs) {
        int w = 0, h = 0;
        TTF_SizeUTF8(s.font, s.text.c_str(), &w, &h);
        total += w;
    }
    return total;
}

void DrawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color)
{
    if (!gFont) return;

    auto segs = SplitSegments(text);
    int curX = x;
    for (auto& seg : segs) {
        SDL_Surface* surf = TTF_RenderUTF8_Blended(seg.font, seg.text.c_str(), color);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect rect = { curX, y, surf->w, surf->h };
        SDL_RenderCopy(renderer, tex, nullptr, &rect);
        curX += surf->w;
        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surf);
    }
}

void DrawTextCenter(SDL_Renderer* renderer, const std::string& text, int y, SDL_Color color)
{
    if (!gFont) return;

    auto segs = SplitSegments(text);
    int textW = TotalWidth(segs);
    int x = (SCREEN_WIDTH - textW) / 2;

    int curX = x;
    for (auto& seg : segs) {
        SDL_Surface* surf = TTF_RenderUTF8_Blended(seg.font, seg.text.c_str(), color);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect rect = { curX, y, surf->w, surf->h };
        SDL_RenderCopy(renderer, tex, nullptr, &rect);
        curX += surf->w;
        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surf);
    }
}

int GetTextWidth(const std::string& text)
{
    if (!gFont) return 0;
    return TotalWidth(SplitSegments(text));
}

void DrawTextInRect(SDL_Renderer* renderer, const std::string& text, SDL_Rect rect, SDL_Color color)
{
    if (!gFont) return;

    auto segs = SplitSegments(text);
    int textW = TotalWidth(segs);
    int textH = 0;
    for (auto& seg : segs) {
        int w = 0, h = 0;
        TTF_SizeUTF8(seg.font, seg.text.c_str(), &w, &h);
        if (h > textH) textH = h;
    }

    int x = rect.x + (rect.w - textW) / 2;
    int y = rect.y + (rect.h - textH) / 2;

    int curX = x;
    for (auto& seg : segs) {
        SDL_Surface* surf = TTF_RenderUTF8_Blended(seg.font, seg.text.c_str(), color);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect dst = { curX, y, surf->w, surf->h };
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        curX += surf->w;
        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surf);
    }
}
