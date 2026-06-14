#pragma execution_character_set("utf-8")
#include "HelpScreen.hpp"
#include "GameState.hpp"
#include "QuizStage.hpp"
#include "TextRenderer.hpp"

//조작법 및 게임 목표를 텍스트로 오버레이 출력하는 함수
void DrawHelpScreen(SDL_Renderer* renderer)
{
    // 반투명 검은색 배경으로 게임 화면 덮기
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);

    SDL_Rect bg = { 0, 0, 800, 600 };
    SDL_RenderFillRect(renderer, &bg);

    //조작 가이드 타이틀 및 목록 출력
    DrawTextCenter(renderer, "조작법", 70);
    DrawTextCenter(renderer, "W A S D : 이동", 140);
    DrawTextCenter(renderer, "마우스 : 조준", 175);
    DrawTextCenter(renderer, "SPACE : 공격", 210);
    DrawTextCenter(renderer, "M : 전체맵", 245);
    DrawTextCenter(renderer, "ESC : 일시정지", 280);

    //승리 조건 목표 출력
    DrawTextCenter(renderer, "목표", 350);
    DrawTextCenter(renderer, "보스를 물리치고", 400);
    DrawTextCenter(renderer, "폐연구소에서 탈출하라!", 440);
}

//아무 키나 누르면 도움말 창을 닫고 일반 게임 상태로 전환
void HandleHelpEvent(SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN) {
        gGameState = GAME_NORMAL;
    }
}
