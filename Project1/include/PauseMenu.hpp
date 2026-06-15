#pragma once

#include <SDL.h>
#include "Player.hpp"

void DrawPauseMenu(SDL_Renderer* renderer, const PlayerData* player);

bool PauseResumeClicked(int x, int y);
bool PauseHelpClicked(int x, int y);
bool PauseQuitClicked(int x, int y);
