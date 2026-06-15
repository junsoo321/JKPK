#pragma once

#include <SDL.h>

void DrawTitleScreen(SDL_Renderer* renderer);
bool TitleStartClicked(int x, int y);
bool TitleSettingsClicked(int x, int y);
bool TitleExitClicked(int x, int y);
