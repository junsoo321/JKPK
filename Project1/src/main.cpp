#include <SDL.h>
#include "Constants.h"
#include "MapSystem.h"
#include "Player.hpp"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Enemy.h"
#include "Boss.hpp"
#include <stdio.h>
#include <stdlib.h>

extern "C" {
    extern int worldMap[MAP_ROWS][MAP_COLS];
    extern int currentRoomX;
    extern int currentRoomY;
}

int bossMap[MAP_ROWS][MAP_COLS];
float gShakeAmount = 0.0f;

auto main(int argc, char* argv[]) -> int
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*     window   = SDL_CreateWindow("Game Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer*   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    InitMap();
    LoadAllImages(renderer);

    PlayerData player;
    InitPlayer(&player);

    BossData mainBoss;
    bool isBossFight = false;
    bool isRunning = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    while (isRunning) {
        //DeltaTime 계산 (이전 프레임과 현재 프레임 사이의 시간 간격)
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; //밀리초를 초 단위로 변환
        lastTime = currentTime;

        if (deltaTime > 0.05f) deltaTime = 0.05f;

        //키보드 입력 처리
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE && event.key.repeat == 0) {
                    FireProjectile(player.x, player.y, deltaTime);
                }

                if (event.key.keysym.sym == SDLK_b && !isBossFight) {
                    isBossFight = true;
                    currentRoomX = BOSS_ROOM_X;
                    currentRoomY = BOSS_ROOM_Y;
                    InitBoss(&mainBoss);

                    for (int r = 0; r < MAP_ROWS; r++) {
                        for (int c = 0; c < MAP_COLS; c++) {
                            worldMap[r][c] = (r == 0 || r == MAP_ROWS - 1 || c == 0 || c == MAP_COLS - 1) ? 1 : 0;
                        }
                    }
                    player.x = SCREEN_WIDTH / 2.0f;
                    player.y = SCREEN_HEIGHT - (TILE_SIZE * 9.0f);
                }
            }
        }


        //화면 흔들림 감쇠
        if (gShakeAmount > 0) {
            gShakeAmount -= SHAKE_DECAY * deltaTime;
            if (gShakeAmount < 0) gShakeAmount = 0;
        }

        const Uint8* keyState = SDL_GetKeyboardState(NULL);

        //플레이어 이동
        UpdatePlayer(&player, keyState, deltaTime);

        if (isBossFight) {
            UpdateBoss(&mainBoss, &player, deltaTime);

            //보스 피격 연출 타이머 업데이트
            if (mainBoss.hitTimer > 0) {
                mainBoss.hitTimer -= deltaTime;
            }

            //투사체 충돌 로직
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (bullets[i].active && bullets[i].owner == 0) {
                    SDL_Rect bRect = { (int)bullets[i].x, (int)bullets[i].y, PROJECTILE_SIZE, PROJECTILE_SIZE };
                    if (!mainBoss.isInvincible && SDL_HasIntersection(&bRect, &mainBoss.drawRect)) {
                        mainBoss.hp -= PLAYER_BULLET_DAMAGE;
                        if (mainBoss.hp < 0) mainBoss.hp = 0;
                        mainBoss.visualHp = (float)mainBoss.hp;
                        mainBoss.hitTimer = 0.1f; //피격 연출 지속시간
                        bullets[i].active = false;
                    }
                }
            }
        }

        //피격 무적 시간 체크 (SDL_GetTicks 기반)
        if (CheckCollision(&player, bullets, &mainBoss)) {
            Uint32 now = SDL_GetTicks();
            if (!player.isInvincible || now > player.invincibleEndTime) {
                player.hp -= 10;
                player.isInvincible = true;
                player.invincibleEndTime = now + BOSS_PLAYER_INVINCIBLE_TIME;
            }
        }

        //렌더링 파트
        int offsetX = 0, offsetY = 0;
        if (gShakeAmount > 0) {
            offsetX = (rand() % (int)(gShakeAmount * 2 + 1)) - (int)gShakeAmount;
            offsetY = (rand() % (int)(gShakeAmount * 2 + 1)) - (int)gShakeAmount;
        }

        SDL_SetRenderDrawColor(renderer, BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, 255);
        SDL_RenderClear(renderer);

        SDL_Rect shakeViewport = { offsetX, offsetY, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderSetViewport(renderer, &shakeViewport);

        DrawMap(renderer, gMapTexture, gWallTexture, gBorderTexture);

        //투사체 이동 및 그리기
        UpdateAndDrawProjectiles(renderer, deltaTime);

        if (isBossFight) {
            DrawBoss(renderer, &mainBoss);
        }
        else {
            UpdateAndDrawEnemies(renderer, player.x, player.y, gEnemyTexture, deltaTime);
            CheckEnemyCollision(bullets);
        }

        DrawPlayer(renderer, &player);

        SDL_Rect normalViewport = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderSetViewport(renderer, &normalViewport);

        if (!isBossFight) DrawMiniMap(renderer);

        SDL_RenderPresent(renderer);
    }

    FreeAllImages();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
