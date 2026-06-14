#include "QuizStage.hpp"
#include "ImageManager.hpp"
#include "QuizQuestions.hpp"
#include "TextRenderer.hpp"
#include <SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <set>

QuizData gQuiz;
extern TTF_Font* gFont;
static std::set<int> usedQuestions; // 이미 출제된 문제의 인덱스 기억

// 대문자를 소문자로 변환
static std::string ToLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

//대소문자 구분 및 띄어쓰기 오차로 인한 오답 방지용 정규화 함수
static std::string Normalize(std::string str)
{
    str = ToLower(str);
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
    return str;
}

//사용하지 않은 문제를 추출하고, 모든 문제를 풀었을 시 set을 리셋하여 랜덤 스폰
void StartQuizStage()
{
    gQuiz.finished = false;
    gQuiz.correct = false;
    gQuiz.showResult = false;
    gQuiz.resultText.clear();
    gQuiz.resultStartTime = 0;

    std::vector<int> available;
    for (int i = 0; i < (int)gQuizQuestions.size(); i++) {
        if (usedQuestions.find(i) == usedQuestions.end()) {
            available.push_back(i);
        }
    }

    // 모든 문제를 풀었다면 중복 체크 목록 초기화
    if (available.empty()) {
        usedQuestions.clear();
        for (int i = 0; i < (int)gQuizQuestions.size(); i++) {
            available.push_back(i);
        }
    }

    int selected = available[rand() % available.size()];
    usedQuestions.insert(selected);

    gQuiz.currentQuestionIndex = selected;
    gQuiz.question = gQuizQuestions[selected].question;
    gQuiz.answers = gQuizQuestions[selected].answers;
    gQuiz.userInput.clear();

    SDL_StartTextInput(); // SDL 한글/영문 텍스트 스트림 입력 활성화
}

//백스페이스 지우기 처리 및 엔터 입력 시 복수 정답 후보군 비교 판정
void HandleQuizEvent(SDL_Event& event)
{
    if (gQuiz.showResult) return;

    // 텍스트 실시간 타이핑 스트림 누적
    if (event.type == SDL_TEXTINPUT) {
        gQuiz.userInput += event.text.text;
    }

    if (event.type == SDL_KEYDOWN) {
        // 백스페이스 글자 삭제
        if (event.key.keysym.sym == SDLK_BACKSPACE) {
            if (!gQuiz.userInput.empty()) {
                gQuiz.userInput.pop_back();
            }
        }

        // 엔터 키 제출 판정
        if (event.key.keysym.sym == SDLK_RETURN) {
            std::string input = Normalize(gQuiz.userInput);

            // 벡터 내 저장된 정답 양식 중 하나라도 일치하면 정답 처리
            for (const auto& answer : gQuiz.answers) {
                if (input == Normalize(answer)) {
                    std::cout << "[QUIZ] CORRECT" << std::endl;
                    gQuiz.correct = true;
                    gQuiz.showResult = true;
                    gQuiz.resultText = "Correct!";
                    gQuiz.resultColor = { 0, 255, 0, 255 }; // 초록
                    gQuiz.resultStartTime = SDL_GetTicks();
                    SDL_StopTextInput();
                    return;
                }
            }

            // 오답 처리 및 답 출력 (answers[0]번 기본 정답 가이드 노출)
            std::cout << "[QUIZ] WRONG" << std::endl;
            gQuiz.correct = false;
            gQuiz.showResult = true;
            gQuiz.resultText = "Wrong! Answer: " + gQuiz.answers[0];
            gQuiz.resultColor = { 255, 80, 80, 255 }; // 빨강
            gQuiz.resultStartTime = SDL_GetTicks();
            SDL_StopTextInput();
        }
    }
}

//퀴즈 인터페이스 오버레이 박스 및 입력창 실시간 표기
void DrawQuizStage(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);

    SDL_Rect overlay = { 0, 0, 800, 600 };
    SDL_RenderFillRect(renderer, &overlay);

    // 중앙 퀴즈 보드 프레임
    SDL_Rect box = { 150, 180, 500, 240 };
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &box);

    DrawText(renderer, "Programming Quiz", 200, 210);
    DrawText(renderer, gQuiz.question, 200, 260);
    DrawText(renderer, "Answer:", 200, 320);
    DrawText(renderer, gQuiz.userInput, 300, 320); // 플레이어가 입력 중인 문자열

    if (gQuiz.showResult) {
        DrawText(renderer, gQuiz.resultText, 200, 370, gQuiz.resultColor);
    }
}

//결과 창 유예 시간(2초) 타이머 트리거 후 스테이지 종료
void UpdateQuizStage()
{
    if (!gQuiz.showResult) return;

    if (SDL_GetTicks() - gQuiz.resultStartTime > 2000) {
        gQuiz.finished = true;
    }
}

bool IsQuizFinished() { return gQuiz.finished; }
bool WasQuizCorrect() { return gQuiz.correct; }
