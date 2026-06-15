#pragma once
#include <SDL.h>

void DrawGameOverScreen(SDL_Renderer* renderer);
bool GameOverRestartClicked(int x, int y);
