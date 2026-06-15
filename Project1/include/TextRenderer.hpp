#pragma once

#include <SDL.h>
#include <string>

void DrawText(
    SDL_Renderer* renderer,
    const std::string& text,
    int x,
    int y,
    SDL_Color color =
    {
        255,
        255,
        255,
        255
    }
);

void DrawTextCenter(
    SDL_Renderer* renderer,
    const std::string& text,
    int y,
    SDL_Color color =
    {
        255,
        255,
        255,
        255
    }
);

int GetTextWidth(const std::string& text);

void DrawTextInRect(
    SDL_Renderer* renderer,
    const std::string& text,
    SDL_Rect rect,
    SDL_Color color =
    {
        255,
        255,
        255,
        255
    }
);
