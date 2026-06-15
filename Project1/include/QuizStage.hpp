#pragma once

#include <SDL.h>
#include <string>

struct QuizData
{
    bool   finished;
    bool   correct;
    int    currentQuestionIndex;
    std::string question;
    std::string options[4]; // A, B, C, D
    int    correctIndex;
    bool   showResult;
    Uint32 resultStartTime;
    std::string resultText;
    SDL_Color   resultColor;
};

extern QuizData gQuiz;

void StartQuizStage();
void HandleQuizEvent(SDL_Event& event);
void DrawQuizStage(SDL_Renderer* renderer);
void UpdateQuizStage();
bool IsQuizFinished();
bool WasQuizCorrect();
