#pragma once
#include "Projectile.hpp"
#include "Player.hpp"
#include "Boss.hpp"

extern float gShakeAmount;
extern Projectile bullets[MAX_PROJECTILES];

void FireBossAngle(float startX, float startY, float angleDegree);
void Pattern_Intro(BossData* b, float deltaTime);
void Pattern_Idle(BossData* b, PlayerData* player, Uint32 currentTime, float deltaTime);
void Pattern_TeleportOut(BossData* b, float deltaTime);
void Pattern_TeleportIn(BossData* b, Uint32 currentTime, float deltaTime);
void Pattern_MeleeAttack(BossData* b, PlayerData* player, float deltaTime);
void Pattern_Phase1to2(BossData* b, float deltaTime);
void Pattern_LaserAndDelay(BossData* b, PlayerData* player, Uint32 currentTime, float deltaTime);
void Pattern_Phase2Intro(BossData* b, float deltaTime);
void Pattern_SwayRoad(BossData* b, float deltaTime);
void Pattern_Phase3Intro(BossData* b, PlayerData* player, float deltaTime);
void Pattern_Phase3Main(BossData* b, PlayerData* player, float deltaTime);
void Pattern_Phase3Outro(BossData* b, PlayerData* player, float deltaTime);
