#pragma execution_character_set("utf-8")
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
#include "SettingsScreen.hpp"
#include "GameOverScreen.hpp"
#include "Item.hpp"
#ifdef _DEBUG
#include "DebugMenu.hpp"
#endif
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

static SDL_Texture* gTransitionTex   = nullptr;
static float        gTransitionTimer = 0.0f;
static bool         gNeedCapture     = false;
static const float  MAZE_TRANS_DUR   = 1.5f;

// 퀴즈 선택 프롬프트 오버레이 레이아웃
static const SDL_Rect QUIZ_OVERLAY   = { 150, 210, 500, 200 };
static const SDL_Rect QUIZ_YES_BTN   = { 210, 355, 160,  45 };
static const SDL_Rect QUIZ_NO_BTN    = { 430, 355, 160,  45 };

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
                    if (gGameState == GAME_NORMAL)   gGameState = GAME_PAUSE;
                    else if (gGameState == GAME_PAUSE)    gGameState = GAME_NORMAL;
                    else if (gGameState == GAME_SETTINGS) gGameState = GAME_TITLE;
                }
                else if (event.key.keysym.sym == SDLK_m) {
                    gShowFullMap = !gShowFullMap;
                }
            }

            //타이틀 스크린 마우스 클릭 처리
            if (gGameState == GAME_TITLE && event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x;
                int my = event.button.y;
                if (TitleStartClicked(mx, my))    gGameState = GAME_HELP;
                if (TitleSettingsClicked(mx, my)) { InitSettingsScreen(); gGameState = GAME_SETTINGS; }
                if (TitleExitClicked(mx, my))     isRunning = false;
            }

            if (gGameState == GAME_SETTINGS) {
                HandleSettingsEvent(event);
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mx = event.button.x, my = event.button.y;
                    if (SettingsBackClicked(mx, my)) gGameState = GAME_TITLE;
                }
            }

            //일시정지(PAUSE) 메뉴 마우스 클릭 처리
            if (gGameState == GAME_PAUSE && event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x;
                int my = event.button.y;
                if (PauseResumeClicked(mx, my)) gGameState = GAME_NORMAL;
                if (PauseHelpClicked(mx, my)) gGameState = GAME_HELP;
                if (PauseQuitClicked(mx, my)) gGameState = GAME_TITLE;
            }

            // 게임오버 — 다시하기 버튼
            if (gGameState == GAME_OVER &&
                event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (GameOverRestartClicked(event.button.x, event.button.y)) {
                    InitRoomNodes();
                    GenerateDungeon();
                    InitMap();
                    InitPlayer(&player);
                    InitProjectiles();
                    ResetEnemies();
                    isBossFight    = false;
                    gShakeAmount   = 0.0f;
                    gGameState     = GAME_HELP;
                }
            }

            //도움말 및 퀴즈 단계 이벤트 전달
            if (gGameState == GAME_HELP) HandleHelpEvent(event);
            if (gGameState == GAME_QUIZ) HandleQuizEvent(event);

            // 퀴즈 선택 프롬프트: Yes=도전 No=포기(즉시 클리어)
            if (gGameState == GAME_QUIZ_PROMPT &&
                event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                auto inBtn = [](int x, int y, const SDL_Rect& r) {
                    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
                };
                if (inBtn(mx, my, QUIZ_YES_BTN)) {
                    StartQuizStage();
                    gGameState = GAME_QUIZ;
                }
                else if (inBtn(mx, my, QUIZ_NO_BTN)) {
                    currentRoom->specialCleared = true;
                    gGameState = GAME_NORMAL;
                }
            }
#ifdef _DEBUG
            if (gGameState == GAME_NORMAL || gGameState == GAME_BOSS || gGameState == GAME_MAZE)
                DebugMenuHandleEvent(event);
#endif
        }

        //화면 흔들림 효과 감쇠 처리
        if (gShakeAmount > 0) {
            gShakeAmount -= SHAKE_DECAY * deltaTime;
            if (gShakeAmount < 0) gShakeAmount = 0;
        }

        //일시정지 상태가 아닐 때만 게임 월드(플레이어, 적, 스테이지) 업데이트
        if (gGameState != GAME_PAUSE) {
            const Uint8* keyState = SDL_GetKeyboardState(NULL);

#ifdef _DEBUG
            {
                DebugAction dbgAct = DebugMenuGetAction();
                if (dbgAct != DEBUG_ACTION_NONE) {
                    switch (dbgAct) {
                    case DEBUG_ACTION_MAP_BOSS:
                        isBossFight = false;
                        currentRoom->roomType = ROOM_BOSS;
                        gGameState = GAME_NORMAL;
                        break;
                    case DEBUG_ACTION_MAP_MAZE:
                        isBossFight = false;
                        currentRoom->roomType = ROOM_MAZE;
                        currentRoom->specialCleared = false;
                        LoadEnemiesForRoom(currentRoomX, currentRoomY);
                        StartMazeStage();
                        gGameState = GAME_MAZE;
                        break;
                    case DEBUG_ACTION_MAP_QUIZ:
                    {
                        isBossFight = false;
                        currentRoom->roomType = ROOM_QUIZ;
                        currentRoom->specialCleared = false;
                        LoadEnemiesForRoom(currentRoomX, currentRoomY);
                        gGameState = GAME_NORMAL;

                        // 플레이어가 컴퓨터 오브젝트와 겹치면 바깥으로 10px 밀어냄
                        SDL_Rect compRect   = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, 100, 100 };
                        SDL_Rect playerRect = { (int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE };
                        if (SDL_HasIntersection(&playerRect, &compRect)) {
                            float compCX = SCREEN_WIDTH  / 2.0f;
                            float compCY = SCREEN_HEIGHT / 2.0f;
                            float dirX = player.x + PLAYER_SIZE / 2.0f - compCX;
                            float dirY = player.y + PLAYER_SIZE / 2.0f - compCY;
                            float len  = sqrtf(dirX * dirX + dirY * dirY);
                            if (len < 0.001f) { dirX = 0.0f; dirY = -1.0f; len = 1.0f; }
                            dirX /= len; dirY /= len;

                            // 겹침이 해소될 때까지 1px씩 밀고, 추가로 10px 여유
                            while (SDL_HasIntersection(&playerRect, &compRect)) {
                                player.x += dirX; player.y += dirY;
                                playerRect.x = (int)player.x; playerRect.y = (int)player.y;
                            }
                            player.x += dirX * 60.0f; player.y += dirY * 60.0f;
                        }
                        break;
                    }
                    case DEBUG_ACTION_MAP_PLAIN:
                        isBossFight = false;
                        currentRoom->roomType = ROOM_NORMAL;
                        currentRoom->specialCleared = false;
                        currentRoom->visited = false;
                        InitMap();
                        LoadEnemiesForRoom(currentRoomX, currentRoomY);
                        gGameState = GAME_NORMAL;
                        break;
                    case DEBUG_ACTION_SPAWN_NORMAL:
                    case DEBUG_ACTION_SPAWN_NINJA:
                    case DEBUG_ACTION_SPAWN_SUICIDE:
                    {
                        EnemyType spawnType = (dbgAct == DEBUG_ACTION_SPAWN_NORMAL) ? ENEMY_NORMAL
                                            : (dbgAct == DEBUG_ACTION_SPAWN_NINJA)  ? ENEMY_NINJA
                                            :                                          ENEMY_SUICIDE;
                        static const float DIRS[8][2] = {
                            { 1.0f, 0.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f }, { 0.0f, -1.0f },
                            { 0.707f, 0.707f }, { -0.707f, 0.707f }, { 0.707f, -0.707f }, { -0.707f, -0.707f }
                        };
                        const float DIST = 200.0f;
                        int startDir = rand() % 8;
                        for (int d = 0; d < 8; d++) {
                            int idx = (startDir + d) % 8;
                            float sx = player.x + DIRS[idx][0] * DIST;
                            float sy = player.y + DIRS[idx][1] * DIST;
                            if (CanMove(sx, sy)) { SpawnEnemyAt(sx, sy, spawnType); break; }
                        }
                        break;
                    }
                    default: break;
                    }
                }
            }
            if (!DebugMenuIsMenuOpen()) {
#endif

             //게임 상태(State)에 따른 플레이어 및 미로 스테이지 이동 업데이트
            if (gGameState == GAME_NORMAL || gGameState == GAME_BOSS) {
                UpdatePlayer(&player, keyState, deltaTime);
            }
            // GAME_QUIZ_PROMPT 중에는 플레이어 이동 없음
            else if (gGameState == GAME_MAZE) {
                UpdateMazeStage(keyState, deltaTime);
            }

            //특수 스테이지(미로/퀴즈 방) 최초 진입 시 상태 초기화
            if (currentRoom->roomType == ROOM_MAZE && !currentRoom->specialCleared &&
                gGameState != GAME_MAZE && gGameState != GAME_MAZE_TRANSITION) {
                gTransitionTimer = 0.0f;
                gNeedCapture     = true;
                gGameState       = GAME_MAZE_TRANSITION;
            }
            if (gGameState == GAME_MAZE_TRANSITION && !gNeedCapture) {
                gTransitionTimer += deltaTime;
                if (gTransitionTimer >= MAZE_TRANS_DUR) {
                    StartMazeStage();
                    gGameState = GAME_MAZE;
                    if (gTransitionTex) { SDL_DestroyTexture(gTransitionTex); gTransitionTex = nullptr; }
                }
            }
            // 퀴즈방: 컴퓨터 오브젝트에 플레이어가 닿으면 선택 프롬프트 표시
            if (currentRoom->roomType == ROOM_QUIZ && !currentRoom->specialCleared &&
                gGameState == GAME_NORMAL) {
                static const SDL_Rect COMPUTER_RECT = {
                    SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, 100, 100
                };
                SDL_Rect playerRect = { (int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE };
                if (SDL_HasIntersection(&playerRect, &COMPUTER_RECT)) {
                    gGameState = GAME_QUIZ_PROMPT;
                }
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
                    } else {
                        int picked = PickUncollectedItem(&player);
                        if (picked != ITEM_NONE) {
                            currentRoom->rewardAvailable = true;
                            currentRoom->tableItemType   = picked;
                        }
                    }
                    currentRoom->specialCleared = true;
                    gGameState = GAME_NORMAL;
                }
            }

            //미로 스테이지 완료 판정
            if (gGameState == GAME_MAZE) {
                if (IsMazeFinished()) {
                    currentRoom->specialCleared = true;
                    int picked = PickUncollectedItem(&player);
                    if (picked != ITEM_NONE) {
                        currentRoom->rewardAvailable = true;
                        currentRoom->tableItemType   = picked;
                    }
                    gGameState = GAME_NORMAL;
                }
            }

            // 일반방 클리어 — 30% 확률, 미획득 아이템만 배치 (한 번만 판정)
            if (gGameState == GAME_NORMAL && currentRoom->roomType == ROOM_NORMAL &&
                !currentRoom->rewardAvailable && !currentRoom->rewardCollected &&
                !AreEnemiesAlive()) {
                currentRoom->rewardCollected = true; // 재판정 방지
                if (rand() % 10 < 3) {
                    int picked = PickUncollectedItem(&player);
                    if (picked != ITEM_NONE) {
                        currentRoom->rewardAvailable = true;
                        currentRoom->rewardCollected = false; // 실제 수령 시 다시 false로
                        currentRoom->tableItemType   = picked;
                    }
                }
            }

            // 탁자 보상 수령 — 탁자에 닿으면 획득 (이동 차단으로 overlap 직전에 멈추므로 4px 확장 판정)
            if (gGameState == GAME_NORMAL && currentRoom->rewardAvailable) {
                SDL_Rect playerR = { (int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE };
                SDL_Rect tableR  = { SCREEN_WIDTH / 2 - 41, SCREEN_HEIGHT / 2 - 41, 83, 83 };
                if (SDL_HasIntersection(&playerR, &tableR)) {
                    ApplyItem(&player, currentRoom->tableItemType);
                    currentRoom->rewardAvailable = false;
                    currentRoom->rewardCollected = true;
                    currentRoom->tableItemType   = ITEM_NONE;
                }
            }

            //플레이어 공격 연사력 타이머 처리 및 투사체 발사
            fireTimer += deltaTime;
            if ((gGameState == GAME_NORMAL || gGameState == GAME_BOSS) && keyState[SDL_SCANCODE_SPACE] && fireTimer >= FIRE_DELAY) {
                FireProjectile(player.x, player.y, deltaTime, player.projectileSpeedMult);
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
#ifdef _DEBUG
            } // !DebugMenuIsMenuOpen()
#endif
        }

        // 플레이어 사망 판정
        if (player.hp <= 0 &&
            (gGameState == GAME_NORMAL || gGameState == GAME_BOSS ||
             gGameState == GAME_QUIZ   || gGameState == GAME_MAZE)) {
            gGameState = GAME_OVER;
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
            DrawMap(renderer, gMapTexture, gWallTexture, gWallTexture, gObstacleTex);
        }
        else if (gGameState == GAME_QUIZ_PROMPT) {
            // 배경 맵 렌더 (팝업 오버레이는 DrawPlayer 이후에 그림)
            int doorMask = 0;
            if (currentRoom->up)    doorMask |= 1;
            if (currentRoom->down)  doorMask |= 2;
            if (currentRoom->left)  doorMask |= 4;
            if (currentRoom->right) doorMask |= 8;
            DrawMap(renderer, GetRoomMapTexture(renderer, doorMask), gWallTexture, gWallTexture, gObstacleTex);

            // 컴퓨터 오브젝트 (on 상태)
            SDL_Rect compRect = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, 100, 100 };
            SDL_Texture* compTex = gComputerOnTex ? gComputerOnTex : gComputerTex;
            if (compTex) SDL_RenderCopy(renderer, compTex, NULL, &compRect);
            else {
                SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
                SDL_RenderFillRect(renderer, &compRect);
            }
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

            bool roomCleared =
                (currentRoom->roomType == ROOM_START) ||
                (currentRoom->roomType == ROOM_NORMAL && !AreEnemiesAlive()) ||
                ((currentRoom->roomType == ROOM_MAZE || currentRoom->roomType == ROOM_QUIZ) &&
                 currentRoom->specialCleared);

            SDL_Texture* bgTex = (currentRoom->roomType == ROOM_BOSS)
                ? gBossMapTexture
                : (roomCleared ? GetRoomMapTextureOpen(renderer, doorMask)
                               : GetRoomMapTexture(renderer, doorMask));
            DrawMap(renderer, bgTex, gWallTexture, gWallTexture, gObstacleTex);
        }

        // 퀴즈방 컴퓨터 오브젝트 렌더 (미클리어 상태의 GAME_NORMAL)
        if (gGameState == GAME_NORMAL &&
            currentRoom->roomType == ROOM_QUIZ && !currentRoom->specialCleared) {
            SDL_Rect compRect = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, 100, 100 };
            if (gComputerTex) SDL_RenderCopy(renderer, gComputerTex, NULL, &compRect);
            else {
                SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
                SDL_RenderFillRect(renderer, &compRect);
            }
        }

        // 보상 탁자 — 클리어 후 맵 중앙에 표시
        if (gGameState == GAME_NORMAL && currentRoom->rewardAvailable) {
            SDL_Rect tableRect = { SCREEN_WIDTH / 2 - 37, SCREEN_HEIGHT / 2 - 37, 75, 75 };
            if (gTableTex) SDL_RenderCopy(renderer, gTableTex, NULL, &tableRect);
            else {
                SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
                SDL_RenderFillRect(renderer, &tableRect);
            }

            // 탁자 위 아이템 (50x50, 책상 표면 기준 정렬)
            int it = currentRoom->tableItemType;
            if (it >= 0 && it < ITEM_COUNT && gItemTextures[it]) {
                SDL_Rect itemRect = { SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 - 37, 50, 50 };
                SDL_RenderCopy(renderer, gItemTextures[it], NULL, &itemRect);
            }
        }

        UpdateAndDrawProjectiles(renderer, deltaTime);

        if (isBossFight) {
            DrawBoss(renderer, &mainBoss);
        }
        else {
            UpdateAndDrawEnemies(renderer, player.x, player.y, deltaTime, &player);
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

        // 퀴즈 스테이지 UI — 플레이어 포함 모든 오브젝트 위에 표시
        if (gGameState == GAME_QUIZ) {
            DrawQuizStage(renderer);
        }

        // 퀴즈 선택 프롬프트 오버레이 — 플레이어 포함 모든 오브젝트 위에 표시
        if (gGameState == GAME_QUIZ_PROMPT) {
            if (gBackplateTex) {
                SDL_SetTextureBlendMode(gBackplateTex, SDL_BLENDMODE_BLEND);
                SDL_RenderCopy(renderer, gBackplateTex, NULL, &QUIZ_OVERLAY);
            } else {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
                SDL_RenderFillRect(renderer, &QUIZ_OVERLAY);
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderDrawRect(renderer, &QUIZ_OVERLAY);
            }

            SDL_Color white = { 255, 255, 255, 255 };
            DrawTextCenter(renderer, "퀴즈에 도전하시겠습니까?", 265, white);

            // Yes / No 버튼
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            auto drawBtn = [&](const SDL_Rect& r, const char* label) {
                bool hov = mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
                if (gButtonTex) {
                    SDL_RenderCopy(renderer, gButtonTex, NULL, &r);
                    if (hov) {
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
                        SDL_RenderFillRect(renderer, &r);
                    }
                } else {
                    SDL_SetRenderDrawColor(renderer, hov ? 120 : 80, hov ? 120 : 80, hov ? 120 : 80, 255);
                    SDL_RenderFillRect(renderer, &r);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderDrawRect(renderer, &r);
                }
                DrawTextInRect(renderer, label, r, white);
            };
            drawBtn(QUIZ_YES_BTN, "Yes");
            drawBtn(QUIZ_NO_BTN,  "No");
        }

        //일시정지 메뉴, 도움말, 타이틀 렌더링
        if (gGameState == GAME_PAUSE) {
            DrawPauseMenu(renderer, &player);
        }
        if (gGameState == GAME_HELP) {
            DrawHelpScreen(renderer);
        }

        if (gGameState == GAME_SETTINGS) {
            SDL_SetRenderDrawColor(renderer, 10, 10, 18, 255);
            SDL_RenderClear(renderer);
            DrawSettingsScreen(renderer);
            SDL_RenderPresent(renderer);
            continue;
        }

        if (gGameState == GAME_TITLE) {
            DrawTitleScreen(renderer);
            SDL_RenderPresent(renderer);
            continue;
        }

        if (gGameState == GAME_OVER) {
            SDL_Rect vp = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
            SDL_RenderSetViewport(renderer, &vp);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            DrawGameOverScreen(renderer);
            SDL_RenderPresent(renderer);
            continue;
        }

        if (gGameState == GAME_MAZE_TRANSITION && !gNeedCapture) {
            SDL_Rect vp = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
            SDL_RenderSetViewport(renderer, &vp);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            float t = gTransitionTimer / MAZE_TRANS_DUR;
            if (t > 1.0f) t = 1.0f;

            if (gTransitionTex) {
                float scale = 1.0f - t;
                if (scale > 0.001f) {
                    int w = (int)(SCREEN_WIDTH  * scale);
                    int h = (int)(SCREEN_HEIGHT * scale);
                    SDL_Rect dst = { SCREEN_WIDTH / 2 - w / 2, SCREEN_HEIGHT / 2 - h / 2, w, h };
                    SDL_RenderCopyEx(renderer, gTransitionTex, NULL, &dst, t * 720.0, NULL, SDL_FLIP_NONE);
                }
            }

            float flicker = fabsf(sinf(gTransitionTimer * 25.0f)) * t;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, (Uint8)(flicker * 180.0f));
            SDL_RenderFillRect(renderer, NULL);

            SDL_RenderPresent(renderer);
            continue;
        }

         //최종 화면 출력
        SDL_Rect normalViewport = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderSetViewport(renderer, &normalViewport);

        if (!isBossFight) DrawMiniMap(renderer);
        if (gGameState != GAME_MAZE && gGameState != GAME_MAZE_TRANSITION) DrawHearts(renderer, &player);

        if (gGameState == GAME_MAZE_TRANSITION && gNeedCapture) {
            SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(
                0, SCREEN_WIDTH, SCREEN_HEIGHT, 24, SDL_PIXELFORMAT_RGB24);
            if (surf) {
                SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGB24, surf->pixels, surf->pitch);
                if (gTransitionTex) SDL_DestroyTexture(gTransitionTex);
                gTransitionTex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_FreeSurface(surf);
            }
            gNeedCapture = false;
        }

#ifdef _DEBUG
        DebugMenuRender(renderer);
#endif
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
