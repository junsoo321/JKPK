#pragma once

#include <SDL.h>

typedef struct PlayerData PlayerData;

// 보스 상태 열거형
typedef enum BossState {
    B_PHASE1_INTRO,       // 등장 연출
    B_IDLE,               // 1페이즈 기본 대기 및 공격
    B_TELEPORT_OUT,       // 텔레포트 사라짐
    B_TELEPORT_IN,        // 텔레포트 나타남
    B_MELEE_ATTACK,       // 근접 회전 공격
    B_PHASE1to2,          // 페이즈 전환 연출
    B_LASER_DELAY,        // 레이저 패턴 전 대기
    B_LASER_PATTERN,      // 레이저 난사 패턴
    B_PHASE2_INTRO,       // 2페이즈 체력 회복 연출
    B_PHASE2_MAIN,        // 2페이즈 탄막 패턴
    B_PHASE3_INTRO,       // 3페이즈 인트로
    B_PHASE3_MAIN,        // 3페이즈 메인
    B_PHASE3_OUTRO,       // 3페이즈 종료(즉사기 패턴)
    B_DEAD                // 보스 사망
} BossState;

//보스 데이터 구조체
typedef struct BossData {
    float x, y;           //현재 좌표
    float startY;         //인트로 시작 좌표
    float targetX, targetY; //이동 목표 좌표
    float rotation;       //출력 회전 각도
    int hp;               //실제 체력
    float visualHp;       //UI 출력용 체력
    float sizeScale;      //크기 배율 (0.0 ~ 1.0)

    BossState state;      //현재 상태
    int phase;            //현재 페이즈 (1 or 2)

    bool isInvincible;    //무적 여부
    bool isWarning;       //근접 공격 경고 상태

    Uint32 lastFireTick;  //마지막 원거리 공격 시간
    float moveTimer;      //텔레포트 쿨타임
    float stateTimer;     //상태 지속 시간 관리
    float meleeCooldown;  //근접 공격 쿨타임

    // 레이저 패턴 관련
    float laserCycle;     //레이저 사이클 타이머
    float laserOffset;    //레이저 생성 위치 오프셋
    bool isVertical;      //레이저 방향 (가로/세로)

    float hitTimer;       //피격 시 빨간색 반짝임 타이머
    SDL_Rect drawRect;    //출력 영역

    float phase3Timer;       //3페이즈 전체용 타이머
    int greenBulletCount;    //남은 초록색 투사체 개수
    float rushWarningTimer;  //돌진 경고 타이머
    float rushAngle;         //돌진 방향 각도
    float rushSpeed;         //돌진 속도
    float rockX, rockY;      //안전지대 바위 위치
    bool isRushing;          //돌진 중 여부
} BossData;

void InitBoss(BossData* b);
void UpdateBoss(BossData* b, PlayerData* player, float deltaTime);
void DrawBoss(SDL_Renderer* renderer, BossData* b);
