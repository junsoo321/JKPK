#pragma once

#include "Constants.h"
#include <stdbool.h>

<<<<<<< HEAD
//ÃÑ ¸Ê ÆÐÅÏÀÇ °³¼ö
#define MAX_PATTERNS 3

//¸Ê µ¥ÀÌÅÍµéÀÌ ´ã±ä 3Â÷¿ø ¹è¿­
extern int worldMap[MAP_ROWS][MAP_COLS]; //ÇöÀç ¸ÊÀÇ »óÅÂ µ¥ÀÌÅÍ, [¼¼·ÎÁÙ][°¡·ÎÁÙ]
extern int mapLayouts[MAX_PATTERNS][MAP_ROWS][MAP_COLS]; //¸Ê ÆÐÅÏ ÀúÀå, [ÆÐÅÏ¹øÈ£][¼¼·ÎÁÙ][°¡·ÎÁÙ]
extern int worldData[MAX_ROOMS_X][MAX_ROOMS_Y][MAP_ROWS][MAP_COLS]; //ÀüÃ¼ ¿ùµåÀÇ »óÅÂ µ¥ÀÌÅÍ, [¸ÊXÁÂÇ¥][¸ÊYÁÂÇ¥]
extern bool visited[MAX_ROOMS_X][MAX_ROOMS_Y]; //¸Ê ¹æ¹® ¿©ºÎ ÀúÀå(ÀÌ¹Ì »ý¼ºµÈ ¹æÀÎÁö ¿©ºÎ), [¸ÊXÁÂÇ¥][¸ÊYÁÂÇ¥]
=======
//ì´ ë§µ íŒ¨í„´ì˜ ê°œìˆ˜
#define MAX_PATTERNS 3

//ë§µ ë°ì´í„°ë“¤ì´ ë‹´ê¸´ 3ì°¨ì› ë°°ì—´
extern int worldMap[MAP_ROWS][MAP_COLS]; //í˜„ìž¬ ë§µì˜ ìƒíƒœ ë°ì´í„°, [ì„¸ë¡œì¤„][ê°€ë¡œì¤„]
extern int mapLayouts[MAX_PATTERNS][MAP_ROWS][MAP_COLS]; //ë§µ íŒ¨í„´ ì €ìž¥, [íŒ¨í„´ë²ˆí˜¸][ì„¸ë¡œì¤„][ê°€ë¡œì¤„]
extern int worldData[MAX_ROOMS_X][MAX_ROOMS_Y][MAP_ROWS][MAP_COLS]; //ì „ì²´ ì›”ë“œì˜ ìƒíƒœ ë°ì´í„°, [ë§µXì¢Œí‘œ][ë§µYì¢Œí‘œ]
extern bool visited[MAX_ROOMS_X][MAX_ROOMS_Y]; //ë§µ ë°©ë¬¸ ì—¬ë¶€ ì €ìž¥(ì´ë¯¸ ìƒì„±ëœ ë°©ì¸ì§€ ì—¬ë¶€), [ë§µXì¢Œí‘œ][ë§µYì¢Œí‘œ]
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
