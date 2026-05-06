#include "Player.hpp"
#include "MapSystem.h"
#include "ImageManager.hpp"
#include <stdio.h>

void InitPlayer(PlayerData* p) {
    p->x = SCREEN_WIDTH / 2.0f;  //시작 X (화면 중앙)
    p->y = SCREEN_HEIGHT / 2.0f; //시작 Y (화면 중앙)
    p->speed = PLAYER_SPEED;    //이동 속도
    p->hp = PLAYER_HP;          //체력
}

//플레이어 이동 함수
void UpdatePlayer(PlayerData* p, const Uint8* keyboardState) {
    //현재 위치(좌표)저장
    float nextX = p->x;
    float nextY = p->y;

    //키보드 입력 받기, 이동할 좌표 저장
    if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP])    nextY -= p->speed; //w, 위
    if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN])  nextY += p->speed; //s, 아래
    if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT])  nextX -= p->speed; //a, 좌
    if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) nextX += p->speed; //d, 우

    //맵 이동 판정 (벽 충돌 보다 먼저(10픽셀) 처리)

    //오른쪽 끝 도달
    if (nextX > SCREEN_WIDTH - 35) {
        if (currentRoomX < MAX_ROOMS_X - 1) { //우측에 맵이 존재할 경우
            MoveToNextRoom(3); //우측 맵으로 이동
            p->x = 50; //플레이어의 x좌표를 왼쪽 벽으로 이동시킴
            return;    //벽 충돌 체크 스킵
        }
    }
    //왼쪽 끝 도달
    else if (nextX < 5) {
        if (currentRoomX > 0) {
            MoveToNextRoom(2);
            p->x = SCREEN_WIDTH - 80;
            return;
        }
    }
    //위쪽 끝 도달
    else if (nextY < 5) {
        if (currentRoomY > 0) {
            MoveToNextRoom(0);
            p->y = SCREEN_HEIGHT - 80;
            return;
        }
    }
    //아래쪽 끝 도달
    else if (nextY > SCREEN_HEIGHT - 35) {
        if (currentRoomY < MAX_ROOMS_Y - 1) {
            MoveToNextRoom(1);
            p->y = 50;
            return;
        }
    }

    //일반 벽 충돌 검사(위의 맵 이동을 거치지 않았을 경우에만 작동)
    bool canMove = true;
    if (IsWall(nextX, nextY) || IsWall(nextX + 28, nextY) || IsWall(nextX, nextY + 28) || IsWall(nextX + 28, nextY + 28)) { //벽이 존재할경우
        canMove = false; //이동불가
    }

    if (canMove) { //벽이 없을경우
        p->x = nextX; //이동할 좌표로 이동
        p->y = nextY;
    }

    //이동 한 좌표 기준 플레이어 출력을 위한 좌표 저장
    p->drawRect.x = (int)p->x;
    p->drawRect.y = (int)p->y;
    p->drawRect.w = PLAYER_SIZE;
    p->drawRect.h = PLAYER_SIZE;
}

void DrawPlayer(SDL_Renderer* renderer, PlayerData* p) {

    if (gPlayerTexture != nullptr) {
        int result = SDL_RenderCopy(renderer, gPlayerTexture, NULL, &p->drawRect);
    }
    else {
        //이미지가 없다면 임시로 노란색 사각형 출력
        SDL_Rect player_rect = { (int)p->x, (int)p->y, PLAYER_SIZE, PLAYER_SIZE };
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &player_rect);
    }
}
