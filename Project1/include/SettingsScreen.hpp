#pragma once
#include <SDL.h>

void InitSettingsScreen();
void DrawSettingsScreen(SDL_Renderer* renderer);
void HandleSettingsEvent(SDL_Event& event);
bool SettingsBackClicked(int x, int y);
