#pragma once

#include "Constants.h"
#include <stdbool.h>

//총 맵 패턴의 개수
#define MAX_PATTERNS 3

//맵 데이터들이 담긴 3차원 배열
extern int worldMap[MAP_ROWS][MAP_COLS]; //현재 맵의 상태 데이터, [세로줄][가로줄]
extern int mapLayouts[MAX_PATTERNS][MAP_ROWS][MAP_COLS]; //맵 패턴 저장, [패턴번호][세로줄][가로줄]
extern int worldData[MAX_ROOMS_X][MAX_ROOMS_Y][MAP_ROWS][MAP_COLS]; //전체 월드의 상태 데이터, [맵X좌표][맵Y좌표]
extern bool visited[MAX_ROOMS_X][MAX_ROOMS_Y]; //맵 방문 여부 저장(이미 생성된 방인지 여부), [맵X좌표][맵Y좌표]
