#pragma once

<<<<<<< HEAD
//ÀÌ¹ÌÁö ºÒ·¯¿À±â ¹× º¯¼ö¿¡ ÇÒ´çÇÏ´Â .hppÆÄÀÏ, SDL ÇÔ¼ö ±¸Á¶¸¦ ¸ô¶ó¼­ Gemini·Î ÀÓ½Ã »ı¼ºÇÔ

#include <SDL.h>

//!!! ÀÌ¹ÌÁö¸¦ ÀúÀåÇÏ´Â Æ÷ÀÎÅÍ º¯¼öµéÀÔ´Ï´Ù.
//!!! ³ªÁß¿¡ ½ÇÁ¦ ÀÌ¹ÌÁö·Î ±³Ã¼ÇÒ ¶§ ÀÌ º¯¼öµé¿¡ ·ÎµåÇÏ¸é µË´Ï´Ù.
extern SDL_Texture* gPlayerTexture; //!!! ÇÃ·¹ÀÌ¾î ¿ÜÇü ÀÌ¹ÌÁö º¯¼ö
extern SDL_Texture* gWallTexture;   //!!! º® Å¸ÀÏ ¿ÜÇü ÀÌ¹ÌÁö º¯¼ö
extern SDL_Texture* gFloorTexture;  //!!! ¹Ù´Ú Å¸ÀÏ ¿ÜÇü ÀÌ¹ÌÁö º¯¼ö
extern SDL_Texture* gEnemyTexture;
extern SDL_Texture* gProjectileTexture;
extern SDL_Texture* gEnemyProjectileTexture;
//!!! ÀÌ¹ÌÁöµéÀ» ºÒ·¯¿À°í ¸Ş¸ğ¸®¿¡¼­ ÇØÁ¦ÇÏ´Â ÇÔ¼öµéÀÔ´Ï´Ù.
=======
//ì´ë¯¸ì§€ ë¶ˆëŸ¬ì˜¤ê¸° ë° ë³€ìˆ˜ì— í• ë‹¹í•˜ëŠ” .hppíŒŒì¼, SDL í•¨ìˆ˜ êµ¬ì¡°ë¥¼ ëª°ë¼ì„œ Geminië¡œ ì„ì‹œ ìƒì„±í•¨

#include <SDL.h>

//!!! ì´ë¯¸ì§€ë¥¼ ì €ì¥í•˜ëŠ” í¬ì¸í„° ë³€ìˆ˜ë“¤ì…ë‹ˆë‹¤.
//!!! ë‚˜ì¤‘ì— ì‹¤ì œ ì´ë¯¸ì§€ë¡œ êµì²´í•  ë•Œ ì´ ë³€ìˆ˜ë“¤ì— ë¡œë“œí•˜ë©´ ë©ë‹ˆë‹¤.
extern SDL_Texture* gPlayerTexture; //!!! í”Œë ˆì´ì–´ ì™¸í˜• ì´ë¯¸ì§€ ë³€ìˆ˜
extern SDL_Texture* gWallTexture;   //!!! ë²½ íƒ€ì¼ ì™¸í˜• ì´ë¯¸ì§€ ë³€ìˆ˜
extern SDL_Texture* gFloorTexture;  //!!! ë°”ë‹¥ íƒ€ì¼ ì™¸í˜• ì´ë¯¸ì§€ ë³€ìˆ˜

//!!! ì´ë¯¸ì§€ë“¤ì„ ë¶ˆëŸ¬ì˜¤ê³  ë©”ëª¨ë¦¬ì—ì„œ í•´ì œí•˜ëŠ” í•¨ìˆ˜ë“¤ì…ë‹ˆë‹¤.
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
void LoadAllImages(SDL_Renderer* renderer);
void FreeAllImages();
