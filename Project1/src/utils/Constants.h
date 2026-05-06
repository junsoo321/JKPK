#pragma once
<<<<<<< HEAD
// ÀüÃ¼ »ó¼ö °ü¸®

//È­¸é Å©±â °ü·Ã ¼³Á¤°ª		***ÇöÀç 800*600 ±âÁØÀ¸·Î ÇÏµåÄÚµù µÇ¾î ÀÖÀ½***
#define SCREEN_WIDTH 800	//°¡·Î ÇØ»óµµ
#define SCREEN_HEIGHT 600	//¼¼·Î ÇØ»óµµ

#define TILE_SIZE 40	//È­¸éÀ» ºÐÇÒÇÒ ±¸¿ªÀÇ Å©±â (40ÀÇ ÀÇ¹Ì : ±¸¿ª 1°³ = 40x40 ÇÈ¼¿)

//À§¿¡¼­ ¼³Á¤ÇÑ È­¸é ÇØ»óµµ¿Í ±¸¿ª Å©±â¸¦ ¹ÙÅÁÀ¸·Î ±¸¿ªÀÇ ÃÑ °³¼ö¸¦ Á¤ÇÔ
#define MAP_ROWS (SCREEN_HEIGHT/TILE_SIZE)	//¼¼·Î ±¸¿ª °³¼ö (¼¼·Î ÇØ»óµµ / ±¸¿ª Å©±â)
#define MAP_COLS (SCREEN_WIDTH/TILE_SIZE)	//°¡·Î ±¸¿ª °³¼ö (°¡·Î ÇØ»óµµ / ±¸¿ª Å©±â)


//·£´ý ¸Ê »ý¼º ½Ã ÀüÃ¼ ¸Ê °³¼öÀÇ ÃÖ´ë°ª
#define MAX_ROOMS_X 5  //°¡·Î ÃÖ´ë °³¼ö
#define MAX_ROOMS_Y 5  //¼¼·Î ÃÖ´ë °³¼ö


//Åõ»çÃ¼ °ü·Ã ¼³Á¤
#define MAX_PROJECTILES 100		//ÇÑ¹ø¿¡ Á¸Àç °¡´ÉÇÑ ÃÑ Åõ»çÃ¼ °³¼ö
#define PROJECTILE_SPEED 0.1f	//Åõ»çÃ¼ ¼Óµµ(³¯¾Æ°¡´Â ¼Óµµ)
#define PROJECTILE_SIZE 8		//Åõ»çÃ¼ Å©±â
#define FIRE_DELAY 500			//°ø°Ý¼Óµµ (ms)

//ÇÃ·¹ÀÌ¾î ¼³Á¤
=======
// ì „ì²´ ìƒìˆ˜ ê´€ë¦¬

//í™”ë©´ í¬ê¸° ê´€ë ¨ ì„¤ì •ê°’		***í˜„ìž¬ 800*600 ê¸°ì¤€ìœ¼ë¡œ í•˜ë“œì½”ë”© ë˜ì–´ ìžˆìŒ***
#define SCREEN_WIDTH 800	//ê°€ë¡œ í•´ìƒë„
#define SCREEN_HEIGHT 600	//ì„¸ë¡œ í•´ìƒë„

#define TILE_SIZE 40	//í™”ë©´ì„ ë¶„í• í•  êµ¬ì—­ì˜ í¬ê¸° (40ì˜ ì˜ë¯¸ : êµ¬ì—­ 1ê°œ = 40x40 í”½ì…€)

//ìœ„ì—ì„œ ì„¤ì •í•œ í™”ë©´ í•´ìƒë„ì™€ êµ¬ì—­ í¬ê¸°ë¥¼ ë°”íƒ•ìœ¼ë¡œ êµ¬ì—­ì˜ ì´ ê°œìˆ˜ë¥¼ ì •í•¨
#define MAP_ROWS (SCREEN_HEIGHT/TILE_SIZE)	//ì„¸ë¡œ êµ¬ì—­ ê°œìˆ˜ (ì„¸ë¡œ í•´ìƒë„ / êµ¬ì—­ í¬ê¸°)
#define MAP_COLS (SCREEN_WIDTH/TILE_SIZE)	//ê°€ë¡œ êµ¬ì—­ ê°œìˆ˜ (ê°€ë¡œ í•´ìƒë„ / êµ¬ì—­ í¬ê¸°)


//ëžœë¤ ë§µ ìƒì„± ì‹œ ì „ì²´ ë§µ ê°œìˆ˜ì˜ ìµœëŒ€ê°’
#define MAX_ROOMS_X 5  //ê°€ë¡œ ìµœëŒ€ ê°œìˆ˜
#define MAX_ROOMS_Y 5  //ì„¸ë¡œ ìµœëŒ€ ê°œìˆ˜


//íˆ¬ì‚¬ì²´ ê´€ë ¨ ì„¤ì •
#define MAX_PROJECTILES 100		//í•œë²ˆì— ì¡´ìž¬ ê°€ëŠ¥í•œ ì´ íˆ¬ì‚¬ì²´ ê°œìˆ˜
#define PROJECTILE_SPEED 0.1f	//íˆ¬ì‚¬ì²´ ì†ë„(ë‚ ì•„ê°€ëŠ” ì†ë„)
#define PROJECTILE_SIZE 8		//íˆ¬ì‚¬ì²´ í¬ê¸°
#define FIRE_DELAY 500			//ê³µê²©ì†ë„ (ms)

//í”Œë ˆì´ì–´ ì„¤ì •
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
#define PLAYER_SIZE 30
#define PLAYER_SPEED 0.1f
#define PLAYER_HP 100

<<<<<<< HEAD
//¸÷(enemy) ¼³Á¤
#define MAX_ENEMIES_PER_ROOM 5  // ¸Ê ´ç »ý¼º °¡´ÉÇÑ ÃÖ´ë ¸÷ ¼ö
#define ENEMY_SPEED 1.0f        // ¸÷ ÀÌµ¿ ¼Óµµ
#define ENEMY_SIZE 30           // ¸÷ Å©±â
//ÃÖ¼Ò, ÃÖ´ë °ø°Ý¼Óµµ ¹üÀ§ ³»ÀÇ ·£´ý ¼ø°£¿¡ °ø°Ý
#define ENEMY_ATTACK_MIN 1500	// ¸÷ ÃÖ¼Ò °ø°Ý¼Óµµ (ms)
#define ENEMY_ATTACK_MAX 4000	// ¸÷ ÃÖ´ë °ø°Ý¼Óµµ (ms)
=======
//ëª¹(enemy) ì„¤ì •
#define MAX_ENEMIES_PER_ROOM 5  // ë§µ ë‹¹ ìƒì„± ê°€ëŠ¥í•œ ìµœëŒ€ ëª¹ ìˆ˜
#define ENEMY_SPEED 1.0f        // ëª¹ ì´ë™ ì†ë„
#define ENEMY_SIZE 30           // ëª¹ í¬ê¸°
//ìµœì†Œ, ìµœëŒ€ ê³µê²©ì†ë„ ë²”ìœ„ ë‚´ì˜ ëžœë¤ ìˆœê°„ì— ê³µê²©
#define ENEMY_ATTACK_MIN 1500	// ëª¹ ìµœì†Œ ê³µê²©ì†ë„ (ms)
#define ENEMY_ATTACK_MAX 4000	// ëª¹ ìµœëŒ€ ê³µê²©ì†ë„ (ms)
>>>>>>> 0f5597ca3a1535c120fd825767f6f74a6b55d4fb
