#include <SDL.h>
#include <SDL_ttf.h>
#include "Constants.h"
#include "MapSystem.hpp"
#include "MapData.hpp"
#include "Player.hpp"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Enemy.hpp"
#include "Boss.hpp"
#include "GameState.hpp"
#include "QuizStage.hpp"
#include "MazeStage.hpp"
#include "PauseMenu.hpp"
#include "HelpScreen.hpp"
#include "TextRenderer.hpp"
#include "TitleScreen.hpp"
#include <iostream>
#include <stdlib.h>
#include <time.h>

extern "C" {
    extern int currentRoomX;
    extern int currentRoomY;
}

float gShakeAmount = 0.0f;
BossData mainBoss;
bool isBossFight = false;

auto main(int argc, char* argv[]) -> int
{
    //SDL 라이브러리 및 게임 시스템 초기화
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    srand((unsigned int)time(NULL));
    SDL_Window* window = SDL_CreateWindow("Game Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //던전 맵 생성 및 이미지 에셋 로드
    InitRoomNodes();
    GenerateDungeon();
    LoadAllImages(renderer);
    LoadCollisionMapsFromImages();
    InitMap();

    SDL_Log("SDL_ttf version : %d", TTF_Linked_Version()->major);

    //플레이어 및 타이머 변수 초기화
    PlayerData player;
    InitPlayer(&player);

    bool isRunning = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    float fireTimer = 0.0f;

    //메인 게임 루프 시작
    while (isRunning) {
        //DeltaTime 계산 및 상한선 제한 (프레임 독립성 유지)
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; //밀리초를 초 단위로 변환
        lastTime = currentTime;
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        //사용자 입력 이벤트 처리 루프 (키보드 및 마우스 클릭)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (gGameState == GAME_NORMAL) gGameState = GAME_PAUSE;
                    else if (gGameState == GAME_PAUSE) gGameState = GAME_NORMAL;
                }
                else if (event.key.keysym.sym == SDLK_m) {
                    gShowFullMap = !gShowFullMap;
                }
            }

            //타이틀 스크린 마우스 클릭 처리
            if (gGameState == GAME_TITLE && event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x;
                int my = event.button.y;
                if (TitleStartClicked(mx, my)) gGameState = GAME_HELP;
                if (TitleExitClicked(mx, my)) isRunning = false;
            }

            //일시정지(PAUSE) 메뉴 마우스 클릭 처리
            if (gGameState == GAME_PAUSE && event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x;
                int my = event.button.y;
                if (PauseResumeClicked(mx, my)) gGameState = GAME_NORMAL;
                if (PauseHelpClicked(mx, my)) gGameState = GAME_HELP;
                if (PauseQuitClicked(mx, my)) gGameState = GAME_TITLE;
            }

            //도움말 및 퀴즈 단계 이벤트 전달
            if (gGameState == GAME_HELP) HandleHelpEvent(event);
            if (gGameState == GAME_QUIZ) HandleQuizEvent(event);
        }

        //화면 흔들림 효과 감쇠 처리
        if (gShakeAmount > 0) {
            gShakeAmount -= SHAKE_DECAY * deltaTime;
            if (gShakeAmount < 0) gShakeAmount = 0;
        }

        //일시정지 상태가 아닐 때만 게임 월드(플레이어, 적, 스테이지) 업데이트
        if (gGameState != GAME_PAUSE) {
            const Uint8* keyState = SDL_GetKeyboardState(NULL);

             //게임 상태(State)에 따른 플레이어 및 미로 스테이지 이동 업데이트
            if (gGameState == GAME_NORMAL || gGameState == GAME_BOSS) {
                UpdatePlayer(&player, keyState, deltaTime);
            }
            else if (gGameState == GAME_MAZE) {
                UpdateMazeStage(keyState, deltaTime);
            }

            //특수 스테이지(미로/퀴즈 방) 최초 진입 시 상태 초기화
            if (currentRoom->roomType == ROOM_MAZE && !currentRoom->specialCleared && gGameState != GAME_MAZE) {
                StartMazeStage();
                gGameState = GAME_MAZE;
            }
            if (currentRoom->roomType == ROOM_QUIZ && !currentRoom->specialCleared && gGameState != GAME_QUIZ) {
                StartQuizStage();
                gGameState = GAME_QUIZ;
            }

            //보스 방 진입 시 보스 데이터 초기화 및 격리벽 생성
            if (!isBossFight && currentRoom->roomType == ROOM_BOSS) {
                isBossFight = true;
                InitBoss(&mainBoss);
                for (int r = 0; r < MAP_ROWS; r++) {
                    for (int c = 0; c < MAP_COLS; c++) {
                        currentRoom->mapData[r][c] = (r == 0 || r == MAP_ROWS - 1 || c == 0 || c == MAP_COLS - 1) ? 1 : 0;
                    }
                }
                player.x = SCREEN_WIDTH / 2.0f;
                player.y = SCREEN_HEIGHT - (TILE_SIZE * 9.0f);
            }

            //퀴즈 스테이지 정오답 결과 판정 및 페널티 부여
            if (gGameState == GAME_QUIZ) {
                UpdateQuizStage();
                if (IsQuizFinished()) {
                    if (!WasQuizCorrect()) {
                        player.hp -= 10;
                        std::cout << "[QUIZ] HP -10" << std::endl;
                        std::cout << "Current HP : " << player.hp << std::endl;
                    }
                    currentRoom->specialCleared = true;
                    gGameState = GAME_NORMAL;
                }
            }

            //미로 스테이지 완료 판정
            if (gGameState == GAME_MAZE) {
                if (IsMazeFinished()) {
                    currentRoom->specialCleared = true;
                    gGameState = GAME_NORMAL;
                }
            }

            //플레이어 공격 연사력 타이머 처리 및 투사체 발사
            fireTimer += deltaTime;
            if ((gGameState == GAME_NORMAL || gGameState == GAME_BOSS) && keyState[SDL_SCANCODE_SPACE] && fireTimer >= FIRE_DELAY) {
                FireProjectile(player.x, player.y, deltaTime);
                fireTimer = 0.0f; //타이머 초기화
            }

            //보스 행동 패턴 업데이트 및 투사체 피격 판정
            if (isBossFight && gGameState != GAME_PAUSE) {
                UpdateBoss(&mainBoss, &player, deltaTime);
                if (mainBoss.hitTimer > 0) {
                    mainBoss.hitTimer -= deltaTime;
                }
                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (bullets[i].active && bullets[i].owner == 0) {
                        SDL_Rect bRect = { (int)bullets[i].x, (int)bullets[i].y, PROJECTILE_SIZE, PROJECTILE_SIZE };
                        if (!mainBoss.isInvincible && SDL_HasIntersection(&bRect, &mainBoss.drawRect)) {
                            mainBoss.hp -= PLAYER_BULLET_DAMAGE;
                            if (mainBoss.hp < 0) mainBoss.hp = 0;
                            mainBoss.visualHp = (float)mainBoss.hp;
                            mainBoss.hitTimer = 0.1f;
                            bullets[i].active = false;
                        }
                    }
                }
            }

            //플레이어 피격 처리 및 무적 타이머 설정
            if (CheckCollision(&player, bullets, &mainBoss)) {
                Uint32 now = SDL_GetTicks();
                if (!player.isInvincible || now > player.invincibleEndTime) {
                    player.hp -= 10;
                    player.isInvincible = true;
                    player.invincibleEndTime = now + BOSS_PLAYER_INVINCIBLE_TIME;
                }
            }
        }

        //화면 렌더링 시작 및 쉐이크 오프셋 계산
        int offsetX = 0, offsetY = 0;
        if (gShakeAmount > 0) {
            offsetX = (rand() % (int)(gShakeAmount * 2 + 1)) - (int)gShakeAmount;
            offsetY = (rand() % (int)(gShakeAmount * 2 + 1)) - (int)gShakeAmount;
        }

        SDL_SetRenderDrawColor(renderer, BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, 255);
        SDL_RenderClear(renderer);

        SDL_Rect shakeViewport = { offsetX, offsetY, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderSetViewport(renderer, &shakeViewport);

        //게임 상태별 월드 배경 및 오브젝트 레이어 렌더링
        if (gGameState == GAME_QUIZ) {
            DrawMap(renderer, gMapTexture, gWallTexture, gWallTexture);
            DrawQuizStage(renderer);
        }
        else if (gGameState == GAME_MAZE) {
            DrawMazeStage(renderer);
        }
        else {
            int doorMask = 0;
            if (currentRoom->up)    doorMask |= 1;
            if (currentRoom->down)  doorMask |= 2;
            if (currentRoom->left)  doorMask |= 4;
            if (currentRoom->right) doorMask |= 8;
            SDL_Texture* bgTex = (currentRoom->roomType == ROOM_BOSS)
                ? gBossMapTexture
                : GetRoomMapTexture(renderer, doorMask);
            DrawMap(renderer, bgTex, gWallTexture, gWallTexture);
        }

        UpdateAndDrawProjectiles(renderer, deltaTime);

        if (isBossFight) {
            DrawBoss(renderer, &mainBoss);
        }
        else {
            UpdateAndDrawEnemies(renderer, player.x, player.y, gEnemyTexture, deltaTime, &player);
            CheckEnemyCollision(bullets);
        }

        if (!gShowFullMap) {
            DrawMiniMap(renderer);
        }
        else {
            DrawMapOverlay(renderer);
        }

        if (gGameState != GAME_MAZE) {
            DrawPlayer(renderer, &player);
        }

        //일시정지 메뉴, 도움말, 타이틀 렌더링
        if (gGameState == GAME_PAUSE) {
            DrawPauseMenu(renderer);
        }
        if (gGameState == GAME_HELP) {
            DrawHelpScreen(renderer);
        }

        if (gGameState == GAME_TITLE) {
            DrawTitleScreen(renderer);
            SDL_RenderPresent(renderer);
            continue;
        }

         //최종 화면 출력
        SDL_Rect normalViewport = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderSetViewport(renderer, &normalViewport);

        if (!isBossFight) DrawMiniMap(renderer);
        DrawHearts(renderer, &player);

        SDL_RenderPresent(renderer);
    }

    //종료 시 메모리 해제 및 SDL 해제
    FreeAllImages();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
