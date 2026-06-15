#pragma once

#include "Constants.h"
#include <stdbool.h>

// 도어 마스크 비트: U=1 D=2 L=4 R=8  →  조합 0~15 (총 16가지)
#define MAX_PATTERNS 16
// 슬롯 = variant * MAX_PATTERNS + doorMask  (variant 0=기본, 1~3=A~C)
#define MAX_COLLISION_SLOTS (MAX_PATTERNS * (NUM_COLLISION_VARIANTS + 1))

#ifdef __cplusplus
extern "C" {
#endif

extern int mapLayouts[MAX_COLLISION_SLOTS][MAP_ROWS][MAP_COLS]; //[슬롯][세로][가로]
extern bool collisionLoaded[MAX_COLLISION_SLOTS];               //해당 슬롯 collision이 로드됐는지
extern int worldData[MAX_ROOMS_X][MAX_ROOMS_Y][MAP_ROWS][MAP_COLS];
extern bool visited[MAX_ROOMS_X][MAX_ROOMS_Y];
extern int bossMapLayout[MAP_ROWS][MAP_COLS];

#ifdef __cplusplus
}
#endif
