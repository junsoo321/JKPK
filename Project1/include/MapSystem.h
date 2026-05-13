#pragma once

#include <SDL.h>

#ifdef __cplusplus //C++에서 이 헤더를 포함해도 C 언어 함수 이름을 그대로 사용할 수 있도록 설정
extern "C" {
#endif

	void InitMap();	 //랜덤하게 맵 패턴을 가져와서 맵을 생성하는 함수
	int IsWall(float x, float y);			//벽 충돌 체크 함수
	void DrawMap(SDL_Renderer* renderer, SDL_Texture* mapBg, SDL_Texture* wallTex, SDL_Texture* borderTex);	//맵 그리기 함수

	//현재 플레이어가 위치한 맵의 좌표
	extern int currentRoomX;
	extern int currentRoomY;

	//맵을 이동하는 함수
	void MoveToNextRoom(int direction); //0:상, 1:하, 2:좌, 3:우

	void DrawMiniMap(SDL_Renderer* renderer); //미니맵 그리는 함수

#ifdef __cplusplus //C++에서 이 헤더를 포함해도 C 언어 함수 이름을 그대로 사용할 수 있도록 설정
}
#endif
