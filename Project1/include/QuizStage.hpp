#pragma once

#include <SDL.h>
#include <string>

void StartQuizStage();
void HandleQuizEvent(SDL_Event* event);
void DrawQuizStage(SDL_Renderer* renderer);
bool IsQuizFinished();
bool IsQuizCorrect();
