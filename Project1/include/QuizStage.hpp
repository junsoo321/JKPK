#pragma once

#include <SDL.h>
#include <string>
#include <vector>

struct QuizData
{
    bool finished;
    bool correct;

    std::string question;

    std::vector<std::string> answers;

    std::string userInput;

    int currentQuestionIndex;

    bool showResult;

    Uint32 resultStartTime;

    std::string resultText;

    SDL_Color resultColor;
};

extern QuizData gQuiz;

void StartQuizStage();

void HandleQuizEvent(
    SDL_Event& event
);

void DrawQuizStage(
    SDL_Renderer* renderer
);

void UpdateQuizStage();

bool IsQuizFinished();

bool WasQuizCorrect();

