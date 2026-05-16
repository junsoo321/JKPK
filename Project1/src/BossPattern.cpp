#include "BossPattern.hpp"
#include "Constants.h"
#include "ImageManager.hpp"
#include "Boss.hpp"
#include "Player.hpp"
#include "Projectile.hpp"
#include <math.h>
#include <stdlib.h>
#include <iostream>

//특정 각도 방향으로 보스의 투사체를 발사하는 함수
void FireBossAngle(float startX, float startY, float angleDegree) { //[시작x좌표][시작y좌표][발사할 각도]
    //각도(Degree)를 삼각함수용 라디안(Radian)으로 변환 (원주율 매크로 M_PI 활용)
    float rad = angleDegree * (M_PI / 180.0f);

    //코사인 함수로 X축 방향 벡터를 구해 시작점에 더함
    float tX = startX + cosf(rad) * 100.0f;

    //사인 함수로 Y축 방향 벡터를 구해 시작점에 더함
    float tY = startY + sinf(rad) * 100.0f;

    //계산된 목표 지점(tX, tY)을 향해 실제 투사체를 생성하고 발사
    FireEnemyProjectile(startX, startY, tX, tY);
}

//2페이즈 탄막 구현 함수
void Pattern_SwayRoad(BossData* b, float deltaTime) {
    static float baseAngle = 90.0f;
    static float fireTimer = 0;
    static float swayTime = 0;
    fireTimer += deltaTime;
    swayTime += deltaTime;
    if (fireTimer >= SWAY_ROAD_FIRE_INTERVAL) {
        float centerX = b->x + (BOSS_SIZE / 2.0f);
        float centerY = b->y + (BOSS_SIZE / 2.0f);
        float swayOffset = sinf(swayTime * SWAY_ROAD_SWAY_SPEED) * SWAY_ROAD_SWAY_RANGE;
        for (int i = 0; i < SWAY_ROAD_BULLET_COUNT; i++) {
            float finalAngle = baseAngle + (i * (360.0f / SWAY_ROAD_BULLET_COUNT)) + swayOffset;
            FireBossAngle(centerX, centerY, finalAngle);
        }
        fireTimer = 0;
    }
}

//보스 첫 등장 인트로
void Pattern_Intro(BossData* b, float deltaTime) {
    b->visualHp += (BOSS_MAX_HP / BOSS_INTRO_TIME) * deltaTime; //체력바가 천천히 차오르는 연출을 위한 변수

    b->isWarning = false;   //인트로 중에는 근접공격 경고 해제

    //보스를 맵 위에서부터 아래로 내려오게 만듬
    if (b->y < b->targetY) {
        b->y += (b->targetY - b->startY) / BOSS_INTRO_TIME * deltaTime;
    }

    //UI용 체력이 최대치에 도달하면 인트로 연출 종료 및 본 게임 상태로 전환
    if (b->visualHp >= BOSS_MAX_HP) {
        b->visualHp = (float)BOSS_MAX_HP; //최대 체력 값으로 고정
        b->y = b->targetY;                 //보스 위치를 최종 위치에 고정
        b->isInvincible = false;           //보스의 무적 해제
        b->state = B_IDLE;                 //대기(IDLE) 상태로 전환
        b->lastFireTick = SDL_GetTicks();  //발사 타이머 초기화
    }
}



//기본 대기 상태
void Pattern_Idle(BossData* b, PlayerData* player, Uint32 currentTime, float deltaTime) {
    //보스 중심점과 플레이어 중심점 사이의 거리 계산
    float dx = (player->x + PLAYER_SIZE / 2) - (b->x + BOSS_SIZE / 2);
    float dy = (player->y + PLAYER_SIZE / 2) - (b->y + BOSS_SIZE / 2);
    float distance = sqrtf(dx * dx + dy * dy);

    //플레이어가 근접 사거리 안에 들어왔을때 경고
    if (distance <= BOSS_MELEE_RANGE) {
        if (b->meleeCooldown <= 0) {              //근접공격 쿨타임이 끝났는지 확인
            b->state = B_MELEE_ATTACK;            //보스 상태를 근접 공격 상태로 전환
            b->stateTimer = BOSS_MELEE_PREP_TIME; //공격 전조(선딜레이) 타이머 설정
            b->isWarning = false;                 //경고 플래그 false
            return;                               //근접 공격 상태 진입 시 아래의 원거리 공격 및 텔포 로직을 스킵
        }
    }

    //근접 사거리의 1.3배 이내에 플레이어가 있고 공격이 가능할 때 노란색 경고 플래그 활성화
    b->isWarning = (distance <= BOSS_MELEE_RANGE * 1.3f && b->meleeCooldown <= 0);

    //원거리 공격 쿨이 지났는지 검사
    if (currentTime - b->lastFireTick > (Uint32)(BOSS_ATTACK_SPEED * 1000.0f)) {
        //플레이어의 좌표를 향해 조준 사격 투사체 생성
        FireEnemyProjectile(b->x + BOSS_SIZE / 2, b->y + BOSS_SIZE / 2, player->x, player->y);
        b->lastFireTick = currentTime; //마지막 발사 시간을 현재 시간으로 갱신하여 쿨타임 리셋
    }

    //다음 텔포까지 남은 시간을 매 프레임 감소
    b->moveTimer -= deltaTime;

    //텔포 쿨이 다 지나면 텔레포트 수행
    if (b->moveTimer <= 0) {
        b->state = B_TELEPORT_OUT;         //보스 상태를 사라지는 상태로 전환
        b->stateTimer = BOSS_MOVE_TP_TIME; //텔레포트 연출 지속 시간 설정

        //텔포할 x좌표 선정
        b->targetX = (float)(100 + rand() % (SCREEN_WIDTH - 200));

        //텔포할 y좌표 선정
        b->targetY = (float)(100 + rand() % (SCREEN_HEIGHT / 2));
    }
}

//텔포 사라지는 연출
void Pattern_TeleportOut(BossData* b, float deltaTime) {
    b->stateTimer -= deltaTime;
    b->sizeScale = b->stateTimer / BOSS_MOVE_TP_TIME;
    if (b->stateTimer <= 0) {
        b->sizeScale = 0.0f;
        b->x = b->targetX;
        b->y = b->targetY;
        b->state = B_TELEPORT_IN;
        b->stateTimer = 0.0f;
    }
}

//텔포 나타나는 연출
void Pattern_TeleportIn(BossData* b, Uint32 currentTime, float deltaTime) {
    b->stateTimer += deltaTime;
    b->sizeScale = b->stateTimer / BOSS_MOVE_TP_TIME;
    if (b->sizeScale >= 1.0f) {
        b->sizeScale = 1.0f;
        b->state = B_IDLE; //대기 상태로 전환
        b->moveTimer = BOSS_MOVE_INTERVAL;
        b->lastFireTick = currentTime;
    }
}

//근접 공격 패턴
void Pattern_MeleeAttack(BossData* b, PlayerData* player, float deltaTime) {
    b->stateTimer -= deltaTime;
    b->rotation += (500.0f) * deltaTime;
    if (b->stateTimer <= 0) {
        if (!player->isInvincible) {
            player->hp -= BOSS_MELEE_DAMAGE;
            player->isInvincible = true;
            player->invincibleEndTime = SDL_GetTicks() + 1000;
        }
        b->state = B_IDLE; //대기 상태로 전환
        b->rotation = 0;
        b->moveTimer = 1.0f;
        b->meleeCooldown = 2.0f;
        b->lastFireTick = SDL_GetTicks();
    }
}

//
void Pattern_Phase1to2(BossData* b, float deltaTime) {
    b->stateTimer -= deltaTime;
    float progress = 1.0f - (b->stateTimer / 1.5f);
    b->rotation += (BOSS_TRANSITION_ROTATION_BASE + progress * BOSS_TRANSITION_ROTATION_ACCEL) * deltaTime;
    b->sizeScale = powf(b->stateTimer / 1.5f, 2.0f);
    if (b->stateTimer <= 0) {
        b->state = B_LASER_DELAY;
        b->stateTimer = 1.0f;
        b->sizeScale = 0.0f;
        b->rotation = 0.0f;
        b->laserCycle = 0.0f;
        b->phase = 2;
    }
}

//레이저 전조(DELAY)와 공격 상태를 묶음
void Pattern_LaserAndDelay(BossData* b, PlayerData* player, Uint32 currentTime, float deltaTime) {
    b->stateTimer -= deltaTime;
    b->sizeScale = 0.0f;

    //전조(Delay) 상태
    if (b->state == B_LASER_DELAY) {
        if (b->stateTimer <= 0) {
            b->state = B_LASER_PATTERN; // 내부 상태 전환
            b->stateTimer = 13.0f;      //레이저 페이즈 지속시간
            b->laserCycle = 0.0f;
            gShakeAmount = 0;
        }
        return;
    }

    //레이저 공격 상태
    b->laserCycle += deltaTime;
    if (b->laserCycle >= BOSS_LASER_WARNING_TIME && b->laserCycle < BOSS_LASER_WARNING_TIME + (deltaTime * 2.0f)) {
        if (b->stateTimer < 10.0f && gShakeAmount <= 0) { //레이저 타이밍에 맞춰서 화면 흔들기
            gShakeAmount = SHAKE_INTENSITY;
        }
    }
    if (b->laserCycle >= BOSS_LASER_CYCLE_TIME) {
        b->laserCycle = 0.0f;
        b->isVertical = rand() % 2;
        b->laserOffset = (float)(rand() % 80);
    }
    //레이저가 켜져있고, 플레이어 무적이 아닐때만 충돌을 검사
    if (b->laserCycle >= BOSS_LASER_WARNING_TIME && b->laserCycle <= BOSS_LASER_ACTIVE_TIME && !player->isInvincible) {

        //레이저를 화면 전체에 간격별로 배치
        for (int i = -BOSS_LASER_STEP; i < SCREEN_WIDTH + BOSS_LASER_STEP; i += BOSS_LASER_STEP) {

            //레이저 위치에 offset을 추가하여 무작위성 적용
            float pos = i + b->laserOffset;
            SDL_Rect lRect;

            //레이저 충돌 박스 생성
            if (b->isVertical) { //세로
                lRect = { (int)pos, 0, BOSS_LASER_THICKNESS, SCREEN_HEIGHT };
            }
            else { //가로
                lRect = { 0, (int)pos, SCREEN_WIDTH, BOSS_LASER_THICKNESS };
            }
            //플레이어 충돌 박스 생성
            SDL_Rect pRect = { (int)player->x, (int)player->y, PLAYER_SIZE, PLAYER_SIZE };

            //AABB로 레이저와 충돌 감지
            if (SDL_HasIntersection(&pRect, &lRect)) {
                player->hp -= 10;
                std::cout << "Player HP : " << (player->hp) << std::endl;
                player->isInvincible = true;
                player->invincibleEndTime = currentTime + 2000;
                break;
            }
        }
    }
    if (b->stateTimer <= 0) { //레이저 패턴 종료
        b->state = B_PHASE2_INTRO; //2페이즈 인트로 상태로 전환
        b->visualHp = 0;
        b->sizeScale = 1.0f;
        b->x = (SCREEN_WIDTH / 2.0f) - (BOSS_SIZE / 2.0f);
        b->y = 100.0f;
    }
}

//2페이즈 인트로 상태
void Pattern_Phase2Intro(BossData* b, float deltaTime) {
    b->isWarning = false;
    b->visualHp += (BOSS_MAX_HP / BOSS_INTRO_TIME) * deltaTime;
    if (b->visualHp >= BOSS_MAX_HP) {
        b->visualHp = (float)BOSS_MAX_HP;
        b->hp = BOSS_MAX_HP;
        b->isInvincible = false;
        b->state = B_PHASE2_MAIN; //2페이즈 메인패턴으로 진입
    }
}

//3페이즈 인트로 상태
void Pattern_Phase3Intro(BossData* b, PlayerData* player, float deltaTime) {
    //패턴 동안 보스 무적 on
    b->isInvincible = true;
    b->isWarning = false;

    //초기화
    if (b->greenBulletCount == -1) {
        b->greenBulletCount = 0;     //화면 내 초록 투사체 카운트 초기화
        b->stateTimer = 0.0f;        //5초 동안 시간 누적할 타이머로 활용
        b->laserCycle = 0.0f;        //지금까지 스폰한 누적 개수 저장용
    }

    //5초에 걸쳐서 50개의 투사체를 랜덤 위치에서 분산 스폰
    if (b->laserCycle < 50.0f) {
        b->stateTimer += deltaTime;

        //5초 동안 50개 비율로 현재 소환되어야 할 목표 누적 개수 계산
        int targetSpawnCount = (int)((b->stateTimer / 5.0f) * 50.0f);
        if (targetSpawnCount > 50) targetSpawnCount = 50;

        float centerX = b->x + (BOSS_SIZE / 2.0f);
        float centerY = b->y + (BOSS_SIZE / 2.0f);

        //목표치에 도달할 때까지 프레임당 분할 소환 (윗 방향은 공간이 적어, 플레이어가 막기 어려움으로 생성x)
        while ((int)b->laserCycle < targetSpawnCount) {
            float spawnX = 0.0f;
            float spawnY = 0.0f;

            int wall = rand() % 3;
            if (wall == 0) { //왼쪽 바깥
                spawnX = -30.0f;
                spawnY = (float)(rand() % SCREEN_HEIGHT);
            }
            else if (wall == 1) { //오른쪽 바깥
                spawnX = (float)SCREEN_WIDTH + 30.0f;
                spawnY = (float)(rand() % SCREEN_HEIGHT);
            }
            else { //아래쪽 바깥
                spawnX = (float)(rand() % SCREEN_WIDTH);
                spawnY = (float)SCREEN_HEIGHT + 30.0f;
            }

            FireProjectile_PHASE3(spawnX, spawnY, centerX, centerY, 320.0f); //초록 투사체로 생성

            b->laserCycle += 1.0f; //스폰 누적 수 증가
            b->greenBulletCount++; //현재 살아있는 개수 증가
        }
    }

    //보스 본체와 초록 투사체의 충돌 검사
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!bullets[i].active || bullets[i].type != 1) continue;

        int currentProjSize = PROJECTILE_SIZE * 2;

        if (bullets[i].x < b->x + BOSS_SIZE &&
            bullets[i].x + currentProjSize > b->x &&
            bullets[i].y < b->y + BOSS_SIZE &&
            bullets[i].y + currentProjSize > b->y) {

            bullets[i].active = false;
            b->greenBulletCount--;

            //투사체 하나당 체력 3% 회복
            b->hp += (int)(BOSS_MAX_HP * 0.03f);
            if (b->hp > BOSS_MAX_HP) b->hp = BOSS_MAX_HP;

            b->visualHp = (float)b->hp;
        }
    }

    //50개 모두 스폰되었고 화면에 남은 초록 투사체가 없으면 패턴 종료
    if (b->laserCycle >= 50.0f && b->greenBulletCount <= 0) {
        b->state = B_PHASE3_MAIN;
        b->rushWarningTimer = 3.0f;
        b->isRushing = false;
        b->greenBulletCount = -1;
    }
}

void Pattern_Phase3Main(BossData* b, PlayerData* player, float deltaTime) {
    //체력이 1% 이하가 되는 순간 즉시 즉사기 패턴으로 강제 전환 및 탈출
    if (b->hp <= (int)(BOSS_MAX_HP * 0.01f)) {
        b->state = B_PHASE3_OUTRO;
        b->stateTimer = 5.0f; //전멸기 시전 시간
        b->rockX = (float)(SCREEN_WIDTH / 2 - 25); //바위 중앙 스폰
        b->rockY = (float)(SCREEN_HEIGHT / 2 + 50);
        b->isRushing = false;  //돌진 상태 플래그 해제
        b->isWarning = false;  //경고선 표시 해제
        return;                //아래의 돌진/추적 연산을 모두 무시하고 즉시 종료
    }

    //초기화
    b->isInvincible = false;
    float centerX = b->x + (BOSS_SIZE / 2.0f);
    float centerY = b->y + (BOSS_SIZE / 2.0f);
    float pCenterX = player->x + (PLAYER_SIZE / 2.0f);
    float pCenterY = player->y + (PLAYER_SIZE / 2.0f);

    //돌진 전 경고 및 추적 단계
    if (!b->isRushing) {
        b->rushWarningTimer -= deltaTime;

        if (b->rushWarningTimer > 1.0f) { //보스 돌진 경로가 플레이어를 따라가도록
            b->rushAngle = atan2f(pCenterY - centerY, pCenterX - centerX);
            b->isWarning = true;
        }
        else if (b->rushWarningTimer <= 1.0f && b->rushWarningTimer > 0.0f) {
            //마지막 1초는 각도 고정
        }
        else {
            b->isRushing = true;
            b->rushSpeed = 600.0f;
            b->isWarning = false;

            if (b->x <= 0) b->x = 5.0f;
            if (b->x >= SCREEN_WIDTH - BOSS_SIZE) b->x = (float)(SCREEN_WIDTH - BOSS_SIZE - 5);
            if (b->y <= 0) b->y = 5.0f;
            if (b->y >= SCREEN_HEIGHT - BOSS_SIZE) b->y = (float)(SCREEN_HEIGHT - BOSS_SIZE - 5);
        }
    }
    //가속 돌진 단계
    else {
        b->rushSpeed += 800.0f * deltaTime; //가속
        b->x += cosf(b->rushAngle) * b->rushSpeed * deltaTime;
        b->y += sinf(b->rushAngle) * b->rushSpeed * deltaTime;

        SDL_Rect bRect = { (int)b->x, (int)b->y, BOSS_SIZE, BOSS_SIZE };
        SDL_Rect pRect = { (int)player->x, (int)player->y, PLAYER_SIZE, PLAYER_SIZE };
        if (SDL_HasIntersection(&bRect, &pRect) && !player->isInvincible) {
            player->hp -= BOSS_MELEE_DAMAGE * 1.5f; //피격시 근접공격 데미지의 1.5배 입힘
            std::cout << "Player HP : " << (player->hp) << std::endl;
            player->isInvincible = true;
            player->invincibleEndTime = SDL_GetTicks() + 1500;
        }

        //벽 충돌 검사
        if (b->x <= 2.0f || b->x >= SCREEN_WIDTH - BOSS_SIZE - 2.0f ||
            b->y <= 2.0f || b->y >= SCREEN_HEIGHT - BOSS_SIZE - 2.0f) {

            if (b->x < 2.0f) b->x = 5.0f;
            if (b->x > SCREEN_WIDTH - BOSS_SIZE - 2.0f) b->x = (float)(SCREEN_WIDTH - BOSS_SIZE - 5);
            if (b->y < 2.0f) b->y = 5.0f;
            if (b->y > SCREEN_HEIGHT - BOSS_SIZE - 2.0f) b->y = (float)(SCREEN_HEIGHT - BOSS_SIZE - 5);

            for (int angle = 0; angle < 360; angle += 15) { //벽 충돌시 사방으로 투사체 발사 (벽 파편 구현)
                FireBossAngle(b->x + BOSS_SIZE / 2.0f, b->y + BOSS_SIZE / 2.0f, (float)angle);
            }

            b->isRushing = false;
            b->rushWarningTimer = 3.0f;
        }
    }
}

//3페이즈 종료 상태(광역 즉사 패턴)
void Pattern_Phase3Outro(BossData* b, PlayerData* player, float deltaTime) {
    b->isInvincible = true;
    b->stateTimer -= deltaTime; //5초 카운트다운

    //보스가 서서히 돌기 시작하면서 점점 빠르게 가속 회전
    float rotationSpeed = (5.0f - b->stateTimer) * 800.0f;
    b->rotation += rotationSpeed * deltaTime;

    //보스를 맵 중앙으로 강제 이동
    float targetCenterX = (SCREEN_WIDTH / 2.0f) - (BOSS_SIZE / 2.0f);
    float targetCenterY = 100.0f;
    b->x += (targetCenterX - b->x) * 2.0f * deltaTime;
    b->y += (targetCenterY - b->y) * 2.0f * deltaTime;

    //타이머 종료
    if (b->stateTimer <= 0) {
        gShakeAmount = 150.0f;

        //피격 판정 계산을 위한 각 객체의 중심 좌표 계산
        float bx = b->x + BOSS_SIZE / 2.0f;
        float by = b->y + BOSS_SIZE / 2.0f;
        float rx = b->rockX + 25.0f;
        float ry = b->rockY + 25.0f;
        float px = player->x + PLAYER_SIZE / 2.0f;
        float py = player->y + PLAYER_SIZE / 2.0f;

        //보스 중심에서 바위 중심을 향하는 상대 위치 벡터 계산
        float bToRx = rx - bx;
        float bToRy = ry - by;

        //보스 중심에서 플레이어 중심을 향하는 상대 위치 벡터 계산
        float bToPx = px - bx;
        float bToPy = py - by;

        //두 벡터의 내적 계산
        float dot = bToRx * bToPx + bToRy * bToPy;

        //피격 여부를 저장할 플래그
        bool safe = false;

        //내적 계산 결과값이 0보다 크다는건 플레이어가 바위가 있는 방향(180도)이내에 있다는 의미
        if (dot > 0) {
            //아크탄젠트(atan2f)를 사용하여 보스-플레이어 사잇각과 보스-바위 사잇각의 실제 라디안 차이를 계산
            float angleDiff = atan2f(bToPy, bToPx) - atan2f(bToRy, bToRx);

            //두 각도의 절대값 차이가 0.2 라디안(약 11도) 미만으로 일직선상에 가깝고, 플레이어가 바위보다 멀다면(바위 뒤에 있다면)
            if (fabs(angleDiff) < 0.2f && (bToPx * bToPx + bToPy * bToPy) > (bToRx * bToRx + bToRy * bToRy)) {
                safe = true;    //생존
            }
        }

        if (!safe) { //생존 실패시
            player->hp -= 9999; // 즉사 데미지
            std::cout << "Player HP : " << (player->hp) << std::endl;
        }

        //보스 사망 처리
        b->state = B_DEAD;
    }
}
