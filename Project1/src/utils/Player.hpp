#pragma once

#include <SDL.h>
#include "Constants.h"

<<<<<<< HEAD
//ÇÃ·¹ÀÌ¾î ±¸Á¶Ã¼
struct PlayerData {
    float x;     //À§Ä¡ ÁÂÇ¥
    float y;
    float speed; //ÀÌµ¿¼Óµµ
    int hp;      //Ã¼·Â  
    SDL_Rect drawRect;  //È÷Æ®¹Ú½º
=======
//í”Œë ˆì´ì–´ êµ¬ì¡°ì²´
struct PlayerData {
    float x;     //ìœ„ì¹˜ ì¢Œí‘œ
    float y;
    float speed; //ì´ë™ì†ë„
    int hp;      //ì²´ë ¥  
    SDL_Rect drawRect;  //ížˆíŠ¸ë°•ìŠ¤
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
};

#ifdef __cplusplus
extern "C" {
#endif
<<<<<<< HEAD
    void InitPlayer(PlayerData* p);     //ÇÃ·¹ÀÌ¾î ÃÊ±â°ª ¼³Á¤
    void UpdatePlayer(PlayerData* p, const Uint8* keyboardState);   //ÀÔ·Â¿¡ µû¸¥ ÀÌµ¿,¸ÊÀÌµ¿,Å×µÎ¸® º® Ãæµ¹ Ã³¸®
    void DrawPlayer(SDL_Renderer* renderer, PlayerData* p); //ÁÂÇ¥¿¡ ÇÃ·¹ÀÌ¾î(È÷Æ®¹Ú½º) Ãâ·Â

#ifdef __cplusplus
}
#endif
=======
    void InitPlayer(PlayerData* p);     //í”Œë ˆì´ì–´ ì´ˆê¸°ê°’ ì„¤ì •
    void UpdatePlayer(PlayerData* p, const Uint8* keyboardState);   //ìž…ë ¥ì— ë”°ë¥¸ ì´ë™,ë§µì´ë™,í…Œë‘ë¦¬ ë²½ ì¶©ëŒ ì²˜ë¦¬
    void DrawPlayer(SDL_Renderer* renderer, PlayerData* p); //ì¢Œí‘œì— í”Œë ˆì´ì–´(ížˆíŠ¸ë°•ìŠ¤) ì¶œë ¥

#ifdef __cplusplus
}
#endif
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
