#pragma once

#include <SDL.h>
#include <stdbool.h>
#include "Constants.h"

<<<<<<< HEAD
//¸÷(Enemy) ±¸Á¶Ã¼ Á¤ÀÇ
=======
//ëª¹(Enemy) êµ¬ì¡°ì²´ ì •ì˜
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
typedef struct {
    float x, y;
    float speed;
    bool active;
<<<<<<< HEAD
    //°ø°Ý ½ÃÁ¡Àº °³º° ÃøÁ¤ÀÌ ¾Æ´Ñ SDL_GetTicks() ±âÁØÀ¸·Î °áÁ¤
    Uint32 lastAttackTime; // ¸¶Áö¸·À¸·Î °ø°ÝÇÑ ½Ã°¢
    Uint32 nextAttackDelay; // ´ÙÀ½ °ø°Ý±îÁö ´ë±âÇÒ ·£´ý ½Ã°£ (ms)
} Enemy;


#ifdef __cplusplus //C++¿¡¼­ ÀÌ Çì´õ¸¦ Æ÷ÇÔÇØµµ C ¾ð¾î ÇÔ¼ö ÀÌ¸§À» ±×´ë·Î »ç¿ëÇÒ ¼ö ÀÖµµ·Ï ¼³Á¤
extern "C" {
#endif
    //¸Ê ÁøÀÔ ½Ã ¸÷ ¼Ó¼º ÃÊ±â°ª ¼³Á¤ ¹× ºÒ·¯¿À´Â ÇÔ¼ö
    void LoadEnemiesForRoom(int roomX, int roomY);

    //¸Ê ÀÌµ¿ ½Ã ÇöÀç Á¸ÀçÇÏ´Â ¸÷µéÀÇ Á¤º¸(À§Ä¡, °³¼ö, ¸¶Áö¸· °ø°Ý½Ã°£, ´ÙÀ½ °ø°Ý½Ã°£)¸¦ ÀúÀåÇÏ´Â ÇÔ¼ö
    void SaveEnemiesForRoom(int roomX, int roomY);

    //¸÷µéÀÌ ¸Ê ¹ÛÀ¸·Î ³ª°¡°Å³ª º®À» Åë°úÇÏ´Â°ÍÀ» ¹æÁöÇÏ´Â ÇÔ¼ö
    void CheckEnemyCollision(void* projectileArray);

    //¸÷ Ãâ·Â ¹× ¾÷µ¥ÀÌÆ®(ÀÌµ¿,°ø°Ý) ÇÔ¼ö
    void UpdateAndDrawEnemies(SDL_Renderer* renderer, float playerX, float playerY);

#ifdef __cplusplus //C++¿¡¼­ ÀÌ Çì´õ¸¦ Æ÷ÇÔÇØµµ C ¾ð¾î ÇÔ¼ö ÀÌ¸§À» ±×´ë·Î »ç¿ëÇÒ ¼ö ÀÖµµ·Ï ¼³Á¤
}
#endif
=======
    //ê³µê²© ì‹œì ì€ ê°œë³„ ì¸¡ì •ì´ ì•„ë‹Œ SDL_GetTicks() ê¸°ì¤€ìœ¼ë¡œ ê²°ì •
    Uint32 lastAttackTime; // ë§ˆì§€ë§‰ìœ¼ë¡œ ê³µê²©í•œ ì‹œê°
    Uint32 nextAttackDelay; // ë‹¤ìŒ ê³µê²©ê¹Œì§€ ëŒ€ê¸°í•  ëžœë¤ ì‹œê°„ (ms)
} Enemy;


#ifdef __cplusplus //C++ì—ì„œ ì´ í—¤ë”ë¥¼ í¬í•¨í•´ë„ C ì–¸ì–´ í•¨ìˆ˜ ì´ë¦„ì„ ê·¸ëŒ€ë¡œ ì‚¬ìš©í•  ìˆ˜ ìžˆë„ë¡ ì„¤ì •
extern "C" {
#endif
    //ë§µ ì§„ìž… ì‹œ ëª¹ ì†ì„± ì´ˆê¸°ê°’ ì„¤ì • ë° ë¶ˆëŸ¬ì˜¤ëŠ” í•¨ìˆ˜
    void LoadEnemiesForRoom(int roomX, int roomY);

    //ë§µ ì´ë™ ì‹œ í˜„ìž¬ ì¡´ìž¬í•˜ëŠ” ëª¹ë“¤ì˜ ì •ë³´(ìœ„ì¹˜, ê°œìˆ˜, ë§ˆì§€ë§‰ ê³µê²©ì‹œê°„, ë‹¤ìŒ ê³µê²©ì‹œê°„)ë¥¼ ì €ìž¥í•˜ëŠ” í•¨ìˆ˜
    void SaveEnemiesForRoom(int roomX, int roomY);

    //ëª¹ë“¤ì´ ë§µ ë°–ìœ¼ë¡œ ë‚˜ê°€ê±°ë‚˜ ë²½ì„ í†µê³¼í•˜ëŠ”ê²ƒì„ ë°©ì§€í•˜ëŠ” í•¨ìˆ˜
    void CheckEnemyCollision(void* projectileArray);

    //ëª¹ ì¶œë ¥ ë° ì—…ë°ì´íŠ¸(ì´ë™,ê³µê²©) í•¨ìˆ˜
    void UpdateAndDrawEnemies(SDL_Renderer* renderer, float playerX, float playerY);

#ifdef __cplusplus //C++ì—ì„œ ì´ í—¤ë”ë¥¼ í¬í•¨í•´ë„ C ì–¸ì–´ í•¨ìˆ˜ ì´ë¦„ì„ ê·¸ëŒ€ë¡œ ì‚¬ìš©í•  ìˆ˜ ìžˆë„ë¡ ì„¤ì •
}
#endif
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
