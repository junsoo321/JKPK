#pragma once

#include <SDL.h>
#include "Constants.h"

//플레이어 구조체
struct PlayerData {
    float x;     //위치 좌표
    float y;
    float speed; //이동속도
    int hp;      //체력  
    SDL_Rect drawRect;  //히트박스
};

#ifdef __cplusplus
extern "C" {
#endif
    void InitPlayer(PlayerData* p);     //플레이어 초기값 설정
    void UpdatePlayer(PlayerData* p, const Uint8* keyboardState);   //입력에 따른 이동,맵이동,테두리 벽 충돌 처리
    void DrawPlayer(SDL_Renderer* renderer, PlayerData* p); //좌표에 플레이어(히트박스) 출력

#ifdef __cplusplus
}
#endif