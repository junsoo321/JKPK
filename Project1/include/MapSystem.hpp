#pragma once

#include <SDL.h>
#include "RoomNode.h"

#ifdef __cplusplus //C++에서 이 헤더를 포함해도 C 언어 함수 이름을 그대로 사용할 수 있도록 설정
extern "C" {
#endif

	void InitMap();
	int  IsWall(float x, float y);
	void DrawMap(SDL_Renderer* renderer, SDL_Texture* mapBg, SDL_Texture* wallTex, SDL_Texture* borderTex);

	// 해당 방향 가장자리 문(type 3)의 중앙 픽셀 좌표 반환 (없으면 -1)
	// direction: 0=상(X), 1=하(X), 2=좌(Y), 3=우(Y)
	int  GetDoorCenter(int direction);

	// 플레이어 히트박스(pos1~pos2)가 해당 방향 가장자리 문(type 3)과 겹치는지 확인
	// direction: 0=상, 1=하, 2=좌, 3=우
	int  HasDoorAtEdge(int direction, float pos1, float pos2);
	int  IsTouchingEdgeDoor(int direction, float x1, float x2, float y1, float y2);

extern int currentRoomX;
	extern int currentRoomY;
	extern RoomNode* currentRoom;
	extern bool gShowFullMap;
	extern RoomNode roomNodes[MAX_ROOMS_X][MAX_ROOMS_Y];

	void MoveToNextRoom(int direction); //0:상, 1:하, 2:좌, 3:우
	void InitRoomNodes();
	void DrawMapOverlay(SDL_Renderer* renderer);	void DrawMiniMap(SDL_Renderer* renderer);
	void InitRoomNodes();
	void GenerateDungeon();

#ifdef __cplusplus //C++에서 이 헤더를 포함해도 C 언어 함수 이름을 그대로 사용할 수 있도록 설정
}
#endif
