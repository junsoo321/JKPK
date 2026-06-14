#pragma execution_character_set("utf-8")
#include "PauseMenu.hpp"
#include "TextRenderer.hpp"

SDL_Rect resumeBtn = { 300, 220, 200, 50 };
SDL_Rect helpBtn = { 300, 290, 200, 50 };
SDL_Rect quitBtn = { 300, 360, 200, 50 };

//일시정지 오버레이 배경, 버튼 렉트, 내부 한글 텍스트 렌더링
void DrawPauseMenu(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // 반투명 암전 효과

    SDL_Rect full = { 0, 0, 800, 600 }; // TODO: SCREEN_WIDTH, SCREEN_HEIGHT 상수가 생기면 교체 권장
    SDL_RenderFillRect(renderer, &full);

    //마우스 상호작용용 버튼 사각형 채우기
    SDL_SetRenderDrawColor(renderer, 90, 90, 90, 255);
    SDL_RenderFillRect(renderer, &resumeBtn);
    SDL_RenderFillRect(renderer, &helpBtn);
    SDL_RenderFillRect(renderer, &quitBtn);

    // UI 시인성 확보를 위한 1px 외곽 테두리선 드로우
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &resumeBtn);
    SDL_RenderDrawRect(renderer, &helpBtn);
    SDL_RenderDrawRect(renderer, &quitBtn);

    // 각 버튼 중앙 정렬 글자 배치 (폰트 세로 오프셋 보정 반영)
    DrawTextCenter(renderer, "계속하기", resumeBtn.y + 12);
    DrawTextCenter(renderer, "조작키", helpBtn.y + 12);
    DrawTextCenter(renderer, "나가기", quitBtn.y + 12);
}

// 입력받은 마우스 좌표(x, y)가 '계속하기' 버튼 영역 내부에 있는지 검증
bool PauseResumeClicked(int x, int y)
{
    return (x >= resumeBtn.x && x <= resumeBtn.x + resumeBtn.w &&
        y >= resumeBtn.y && y <= resumeBtn.y + resumeBtn.h);
}

// 입력받은 마우스 좌표(x, y)가 '조작키' 버튼 영역 내부에 있는지 검증
bool PauseHelpClicked(int x, int y)
{
    return (x >= helpBtn.x && x <= helpBtn.x + helpBtn.w &&
        y >= helpBtn.y && y <= helpBtn.y + helpBtn.h);
}

// 입력받은 마우스 좌표(x, y)가 '나가기' 버튼 영역 내부에 있는지 검증
bool PauseQuitClicked(int x, int y)
{
    return (x >= quitBtn.x && x <= quitBtn.x + quitBtn.w &&
        y >= quitBtn.y && y <= quitBtn.y + quitBtn.h);
}
