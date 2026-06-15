#pragma execution_character_set("utf-8")
#include "QuizStage.hpp"
#include "ImageManager.hpp"
#include "QuizQuestions.hpp"
#include "TextRenderer.hpp"
#include "Constants.h"
#include <SDL_ttf.h>
#include <iostream>
#include <set>

QuizData gQuiz;
static std::set<int> usedQuestions;

// 퀴즈 오버레이 레이아웃
static const SDL_Rect QUIZ_BOX   = { 100, 90, 600, 420 };
static const SDL_Rect QUIZ_BTN_A = { 125, 310, 255, 55 };
static const SDL_Rect QUIZ_BTN_B = { 420, 310, 255, 55 };
static const SDL_Rect QUIZ_BTN_C = { 125, 380, 255, 55 };
static const SDL_Rect QUIZ_BTN_D = { 420, 380, 255, 55 };

static const SDL_Rect* QUIZ_BTNS[4] = { &QUIZ_BTN_A, &QUIZ_BTN_B, &QUIZ_BTN_C, &QUIZ_BTN_D };
static const char* BTN_LABELS[4] = { "A", "B", "C", "D" };

void StartQuizStage()
{
    gQuiz.finished    = false;
    gQuiz.correct     = false;
    gQuiz.showResult  = false;
    gQuiz.resultText.clear();
    gQuiz.resultStartTime = 0;

    std::vector<int> available;
    for (int i = 0; i < (int)gQuizQuestions.size(); i++) {
        if (usedQuestions.find(i) == usedQuestions.end())
            available.push_back(i);
    }
    if (available.empty()) {
        usedQuestions.clear();
        for (int i = 0; i < (int)gQuizQuestions.size(); i++)
            available.push_back(i);
    }

    int selected = available[rand() % available.size()];
    usedQuestions.insert(selected);

    gQuiz.currentQuestionIndex = selected;
    gQuiz.question     = gQuizQuestions[selected].question;
    for (int i = 0; i < 4; i++)
        gQuiz.options[i] = gQuizQuestions[selected].options[i];
    gQuiz.correctIndex = gQuizQuestions[selected].correctIndex;
}

// 버튼 클릭 처리 — 마우스로 A/B/C/D 선택
void HandleQuizEvent(SDL_Event& event)
{
    if (gQuiz.showResult) return;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mx = event.button.x;
        int my = event.button.y;

        for (int i = 0; i < 4; i++) {
            const SDL_Rect& r = *QUIZ_BTNS[i];
            if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) {
                if (i == gQuiz.correctIndex) {
                    std::cout << "[QUIZ] CORRECT" << std::endl;
                    gQuiz.correct     = true;
                    gQuiz.resultText  = "Correct!";
                    gQuiz.resultColor = { 0, 220, 80, 255 };
                } else {
                    std::cout << "[QUIZ] WRONG" << std::endl;
                    gQuiz.correct     = false;
                    gQuiz.resultText  = std::string("Wrong! Answer: ") + BTN_LABELS[gQuiz.correctIndex];
                    gQuiz.resultColor = { 255, 80, 80, 255 };
                }
                gQuiz.showResult      = true;
                gQuiz.resultStartTime = SDL_GetTicks();
                return;
            }
        }
    }
}

// backplate + 4지선다 버튼 렌더링
void DrawQuizStage(SDL_Renderer* renderer)
{
    // 전체 화면 반투명 어둠 깔기
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect fullScreen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &fullScreen);

    // 퀴즈 박스 backplate
    if (gBackplateTex) {
        SDL_SetTextureBlendMode(gBackplateTex, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, gBackplateTex, NULL, &QUIZ_BOX);
    } else {
        SDL_SetRenderDrawColor(renderer, 40, 40, 50, 230);
        SDL_RenderFillRect(renderer, &QUIZ_BOX);
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        SDL_RenderDrawRect(renderer, &QUIZ_BOX);
    }

    SDL_Color white  = { 255, 255, 255, 255 };
    SDL_Color yellow = { 255, 220, 60,  255 };

    // 제목
    DrawTextCenter(renderer, "QUIZ", 130, yellow);

    // 문제 텍스트
    SDL_Rect questionArea = { QUIZ_BOX.x + 20, 175, QUIZ_BOX.w - 40, 110 };
    DrawTextInRect(renderer, gQuiz.question.c_str(), questionArea, white);

    // 구분선
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 180);
    SDL_RenderDrawLine(renderer, QUIZ_BOX.x + 20, 295, QUIZ_BOX.x + QUIZ_BOX.w - 20, 295);

    // 4지선다 버튼
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    for (int i = 0; i < 4; i++) {
        const SDL_Rect& r = *QUIZ_BTNS[i];
        bool hov = !gQuiz.showResult &&
                   mx >= r.x && mx < r.x + r.w &&
                   my >= r.y && my < r.y + r.h;

        if (gButtonTex) {
            SDL_RenderCopy(renderer, gButtonTex, NULL, &r);
            if (hov) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
                SDL_RenderFillRect(renderer, &r);
            }
        } else {
            SDL_SetRenderDrawColor(renderer,
                hov ? 120 : 70, hov ? 120 : 70, hov ? 140 : 90, 255);
            SDL_RenderFillRect(renderer, &r);
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderDrawRect(renderer, &r);
        }

        // "A. 선택지 텍스트"
        std::string label = std::string(BTN_LABELS[i]) + ". " + gQuiz.options[i];
        DrawTextInRect(renderer, label.c_str(), r, white);
    }

    // 결과 텍스트
    if (gQuiz.showResult) {
        DrawTextCenter(renderer, gQuiz.resultText.c_str(), 455, gQuiz.resultColor);
    }
}

void UpdateQuizStage()
{
    if (!gQuiz.showResult) return;
    if (SDL_GetTicks() - gQuiz.resultStartTime > 2000)
        gQuiz.finished = true;
}

bool IsQuizFinished() { return gQuiz.finished; }
bool WasQuizCorrect()  { return gQuiz.correct;  }
