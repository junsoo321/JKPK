#pragma once

#include <SDL.h>

<<<<<<< HEAD
#ifdef __cplusplus //C++¿¡¼­ ÀÌ Çì´õ¸¦ Æ÷ÇÔÇØµµ C ¾ð¾î ÇÔ¼ö ÀÌ¸§À» ±×´ë·Î »ç¿ëÇÒ ¼ö ÀÖµµ·Ï ¼³Á¤
extern "C" {
#endif

	void InitMap();	 //·£´ýÇÏ°Ô ¸Ê ÆÐÅÏÀ» °¡Á®¿Í¼­ ¸ÊÀ» »ý¼ºÇÏ´Â ÇÔ¼ö
	int IsWall(float x, float y);			//º® Ãæµ¹ Ã¼Å© ÇÔ¼ö
	void DrawMap(SDL_Renderer* renderer);	//¸Ê ±×¸®±â ÇÔ¼ö

	//ÇöÀç ÇÃ·¹ÀÌ¾î°¡ À§Ä¡ÇÑ ¸ÊÀÇ ÁÂÇ¥
	extern int currentRoomX;
	extern int currentRoomY;

	//¸ÊÀ» ÀÌµ¿ÇÏ´Â ÇÔ¼ö
	void MoveToNextRoom(int direction); //0:»ó, 1:ÇÏ, 2:ÁÂ, 3:¿ì

	void DrawMiniMap(SDL_Renderer* renderer); //¹Ì´Ï¸Ê ±×¸®´Â ÇÔ¼ö

#ifdef __cplusplus //C++¿¡¼­ ÀÌ Çì´õ¸¦ Æ÷ÇÔÇØµµ C ¾ð¾î ÇÔ¼ö ÀÌ¸§À» ±×´ë·Î »ç¿ëÇÒ ¼ö ÀÖµµ·Ï ¼³Á¤
}
#endif
=======
#ifdef __cplusplus //C++ì—ì„œ ì´ í—¤ë”ë¥¼ í¬í•¨í•´ë„ C ì–¸ì–´ í•¨ìˆ˜ ì´ë¦„ì„ ê·¸ëŒ€ë¡œ ì‚¬ìš©í•  ìˆ˜ ìžˆë„ë¡ ì„¤ì •
extern "C" {
#endif

	void InitMap();	 //ëžœë¤í•˜ê²Œ ë§µ íŒ¨í„´ì„ ê°€ì ¸ì™€ì„œ ë§µì„ ìƒì„±í•˜ëŠ” í•¨ìˆ˜
	int IsWall(float x, float y);			//ë²½ ì¶©ëŒ ì²´í¬ í•¨ìˆ˜
	void DrawMap(SDL_Renderer* renderer);	//ë§µ ê·¸ë¦¬ê¸° í•¨ìˆ˜

	//í˜„ìž¬ í”Œë ˆì´ì–´ê°€ ìœ„ì¹˜í•œ ë§µì˜ ì¢Œí‘œ
	extern int currentRoomX;
	extern int currentRoomY;

	//ë§µì„ ì´ë™í•˜ëŠ” í•¨ìˆ˜
	void MoveToNextRoom(int direction); //0:ìƒ, 1:í•˜, 2:ì¢Œ, 3:ìš°

	void DrawMiniMap(SDL_Renderer* renderer); //ë¯¸ë‹ˆë§µ ê·¸ë¦¬ëŠ” í•¨ìˆ˜

#ifdef __cplusplus //C++ì—ì„œ ì´ í—¤ë”ë¥¼ í¬í•¨í•´ë„ C ì–¸ì–´ í•¨ìˆ˜ ì´ë¦„ì„ ê·¸ëŒ€ë¡œ ì‚¬ìš©í•  ìˆ˜ ìžˆë„ë¡ ì„¤ì •
}
#endif
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
