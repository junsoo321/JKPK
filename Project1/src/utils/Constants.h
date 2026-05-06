#pragma once
// 전체 상수 관리

//화면 크기 관련 설정값		***현재 800*600 기준으로 하드코딩 되어 있음***
#define SCREEN_WIDTH 800	//가로 해상도
#define SCREEN_HEIGHT 600	//세로 해상도

#define TILE_SIZE 40	//화면을 분할할 구역의 크기 (40의 의미 : 구역 1개 = 40x40 픽셀)

//위에서 설정한 화면 해상도와 구역 크기를 바탕으로 구역의 총 개수를 정함
#define MAP_ROWS (SCREEN_HEIGHT/TILE_SIZE)	//세로 구역 개수 (세로 해상도 / 구역 크기)
#define MAP_COLS (SCREEN_WIDTH/TILE_SIZE)	//가로 구역 개수 (가로 해상도 / 구역 크기)


//랜덤 맵 생성 시 전체 맵 개수의 최대값
#define MAX_ROOMS_X 5  //가로 최대 개수
#define MAX_ROOMS_Y 5  //세로 최대 개수


//투사체 관련 설정
#define MAX_PROJECTILES 100		//한번에 존재 가능한 총 투사체 개수
#define PROJECTILE_SPEED 0.1f	//투사체 속도(날아가는 속도)
#define PROJECTILE_SIZE 8		//투사체 크기
#define FIRE_DELAY 500			//공격속도 (ms)

//플레이어 설정
#define PLAYER_SIZE 30
#define PLAYER_SPEED 0.1f
#define PLAYER_HP 100

//몹(enemy) 설정
#define MAX_ENEMIES_PER_ROOM 5  // 맵 당 생성 가능한 최대 몹 수
#define ENEMY_SPEED 1.0f        // 몹 이동 속도
#define ENEMY_SIZE 30           // 몹 크기
//최소, 최대 공격속도 범위 내의 랜덤 순간에 공격
#define ENEMY_ATTACK_MIN 1500	// 몹 최소 공격속도 (ms)
#define ENEMY_ATTACK_MAX 4000	// 몹 최대 공격속도 (ms)
