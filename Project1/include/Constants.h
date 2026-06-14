#pragma once
// 전체 상수 관리

//화면 크기 관련 설정값		***현재 800*600 기준으로 하드코딩 되어 있음***

#define SCREEN_WIDTH 800	//	가로 해상도
#define SCREEN_HEIGHT 600	//	세로 해상도
#define BG_COLOR_R 30       //	배경색 (Red)
#define BG_COLOR_G 30       //	배경색 (Green)
#define BG_COLOR_B 35       //	배경색 (Blue)

#define TILE_SIZE 10	//화면을 분할할 구역의 크기 (10의 의미 : 구역 1개 = 10x10 픽셀)

//위에서 설정한 화면 해상도와 구역 크기를 바탕으로 구역의 총 개수를 정함
#define MAP_ROWS (SCREEN_HEIGHT/TILE_SIZE)	//세로 구역 개수 (세로 해상도 / 구역 크기)
#define MAP_COLS (SCREEN_WIDTH/TILE_SIZE)	//가로 구역 개수 (가로 해상도 / 구역 크기)


//랜덤 맵 생성 시 전체 맵 개수의 최대값
#define MAX_ROOMS_X 10  //가로 최대 개수
#define MAX_ROOMS_Y 10  //세로 최대 개수
//맵 정보
#define ROOM_NORMAL 0
#define ROOM_BOSS 1
#define ROOM_START 2
#define ROOM_MAZE 3
#define ROOM_QUIZ 4

//미로맵
#define MAZE_ROOM_COUNT 3 //전체 맵 중 미로맵의 개수
#define MAZE_TILE_SIZE 32 
#define MAZE_PLAYER_SIZE 24

//퀴즈맵
#define QUIZ_ROOM_COUNT 3 //전채 맵 중 퀴즈맵의 개수

//플레이어 설정
#define PLAYER_SIZE 36				//	히트박스 크기 — 실제 캐릭터 도트 영역(3x 기준 36×42)에 맞춤
#define CHAR_FRAME_SIZE 32			//	스프라이트 시트 한 프레임 크기 (px)
#define PLAYER_RENDER_OFFSET_X 30	//	스프라이트를 히트박스 기준으로 좌측으로 당기는 오프셋 (10px * 3)
#define PLAYER_RENDER_OFFSET_Y 27	//	스프라이트를 히트박스 기준으로 위쪽으로 당기는 오프셋 (9px * 3)
#define PLAYER_SPEED 200.0f
#define PLAYER_FRICTION 8.0f
#define PLAYER_FOOT_OFFSET 20		//	발 히트박스 시작 Y 오프셋 (히트박스 상단 기준)
#define DOOR_SIZE 60				//	출입문 개구부 크기 (px)
#define DOOR_THICKNESS 20			//	출입문 두께 (px, 표시용)
#define PLAYER_HP 100
#define PLAYER_BULLET_DAMAGE 50
#define BOSS_PLAYER_INVINCIBLE_TIME 1000

//플레이어 투사체 관련 설정
#define MAX_PROJECTILES 5000			//	한번에 존재 가능한 총 투사체 개수
#define PROJECTILE_SPEED 400.0f			//	투사체 속도(날아가는 속도)
#define BOSS_PROJECTILE_SPEED 250.0f	//	보스 투사체 속도
#define PROJECTILE_SIZE 32				//	투사체 크기
#define FIRE_DELAY 0.1f					//	공격속도 (ms)
#define PROJECTILE_SPIN_SPEED 360.0f	//	투사체 회전 속도 (도/초)

//몹(enemy) 설정
#define MAX_ENEMIES_PER_ROOM 5  //맵 당 생성 가능한 최대 몹 수
#define ENEMY_SPEED 250.0f		//적 속도
#define SUICIDE_SPEED 400.0f	//자폭몹 속도
#define NINJA_SPEED 320.0f		//닌자몹 속도
#define ENEMY_SIZE 30           //적 크기
#define ENEMY_ATK 10			//적 공격력(투사체)
#define ENEMY_KEEP_DISTANCE 250.0f //일반몹이 플레이어와 유지하려는 거리
#define NINJA_DASH_DISTANCE 180.0f //닌자몹 돌진 공격 사거리
#define NINJA_KEEP_DISTANCE (NINJA_DASH_DISTANCE * 0.7f)	//닌자몹이 플레이어와 유지하려는 거리

//최소, 최대 공격속도 범위 내의 랜덤 순간에 공격
#define ENEMY_ATTACK_MIN 1500	//몹 최소 공격속도 (ms)
#define ENEMY_ATTACK_MAX 4000	//몹 최대 공격속도 (ms)


#define BOSS_SIZE 240            //보스 기본 크기
#define BOSS_MAX_HP 3000         //보스 최대 체력
#define BOSS_INTRO_TIME 2.0f     //등장 애니메이션 시간(초)
#define BOSS_MOVE_INTERVAL 3.0f  //1페이즈 텔레포트 간격(초)
#define BOSS_MOVE_TP_TIME 0.5f   //텔레포트 연출 시간(초)


//보스 공격 패턴 (1페이즈)
#define BOSS_ATTACK_SPEED 1.2f   //원거리 발사 간격(초)
#define BOSS_MELEE_RANGE 150.0f  //근접 공격 발동 범위
#define BOSS_MELEE_PREP_TIME 0.8f//근접 공격 준비 시간(회전 대기 시간)
#define BOSS_MELEE_DAMAGE 20     //근접 공격 데미지
#define BOSS_MELEE_COOLDOWN 2.0f //근접 공격 재사용 대기시간

//페이즈 전환
#define BOSS_TRANSITION_ROTATION_BASE 300.0f  //전환 시 기본 회전 속도
#define BOSS_TRANSITION_ROTATION_ACCEL 1500.0f//전환 시 추가 가속도

//레이저 패턴
#define BOSS_LASER_CYCLE_TIME 2.2f    //레이저 한 사이클 총 시간
#define BOSS_LASER_WARNING_TIME 1.2f  //경고선 유지 시간
#define BOSS_LASER_ACTIVE_TIME 1.8f   //실제 레이저 공격 판정 시간
#define BOSS_LASER_THICKNESS 30       //레이저 두께(px)
#define BOSS_LASER_STEP 90           //레이저 사이의 간격

//2페이즈 탄막 패턴
#define SWAY_ROAD_FIRE_INTERVAL 0.12f //탄막 연사 속도
#define SWAY_ROAD_SWAY_SPEED 1.0f     //좌우 흔들림 속도
#define SWAY_ROAD_SWAY_RANGE 45.0f    //좌우 흔들림 각도 범위
#define SWAY_ROAD_BULLET_COUNT 16      //한 번에 발사되는 탄환 개수 (방사형)

#define SHAKE_MAX_STRENGTH 30.0f  //최대 흔들림 강도
#define SHAKE_DECAY 40.0f         //초당 감소 수치 (높을수록 빨리 멈춤)
#define SHAKE_INTENSITY 30.0f     //레이저 발사 시 기본 흔들림 세기
