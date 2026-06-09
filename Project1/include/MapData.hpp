#pragma once

#include "Constants.h"
#include <stdbool.h>

//총 맵 패턴의 개수
#define MAX_PATTERNS 3

extern int mapLayouts[MAX_PATTERNS][MAP_ROWS][MAP_COLS]; //맵 패턴 저장, [패턴번호][세로줄][가로줄]
