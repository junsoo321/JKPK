#include "MapSystem.hpp"
#include "MapData.hpp"
#include "ImageManager.hpp"
#include "Constants.h"
#include "Projectile.hpp"
#include "Enemy.hpp"
#include "Boss.hpp"
#include "GameState.hpp"
#include "QuizStage.hpp"
#include "MazeStage.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

RoomNode roomNodes[MAX_ROOMS_X][MAX_ROOMS_Y];
extern BossData mainBoss;
extern bool isBossFight;
bool gShowFullMap = false;

//방의 연결 방향으로 도어 마스크 계산 (U=1 D=2 L=4 R=8)
static int CalcDoorMask(RoomNode* room) {
    int mask = 0;
    if (room->up)    mask |= 1;
    if (room->down)  mask |= 2;
    if (room->left)  mask |= 4;
    if (room->right) mask |= 8;
    return mask;
}

//도어 마스크에 맞는 collision + 배경 이미지를 사용해 맵을 생성하는 함수
void InitMap() {
    currentRoom = &roomNodes[currentRoomX][currentRoomY];
    if (currentRoom->visited) return;

    int doorMask = CalcDoorMask(currentRoom);

    // 도어 마스크에 맞는 collision PNG 로드 (없으면 빈 맵)
    LoadRoomCollisionMap(doorMask);

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int tile = 0;

            // 월드 가장자리 경계 벽 설정 (2: 외곽 벽)
            if (r == 0 && currentRoomY == 0) tile = 2;
            if (r == MAP_ROWS - 1 && currentRoomY == MAX_ROOMS_Y - 1) tile = 2;
            if (c == 0 && currentRoomX == 0) tile = 2;
            if (c == MAP_COLS - 1 && currentRoomX == MAX_ROOMS_X - 1) tile = 2;

            //도어 마스크에 해당하는 collision 패턴 적용
            if (r > 0 && r < MAP_ROWS - 1 && c > 0 && c < MAP_COLS - 1) {
                tile = mapLayouts[doorMask][r][c];
            }

            currentRoom->mapData[r][c] = tile;
        }
    }

    currentRoom->visited = true;
}

//플레이어, 몹, 투사체의 벽 충돌 처리 (보스전 격리벽 연산 포함)
int IsWall(float x, float y)
{
    // 보스전이 활성화되어 있다면 화면 밖으로 나가지 못하게 경계 벽 처리
    if (isBossFight && mainBoss.hp > 0) {
        if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return 1;
    }

    int col = (int)(x / TILE_SIZE);
    int row = (int)(y / TILE_SIZE);

    if (row < 0 || row >= MAP_ROWS) return 1;
    if (col < 0 || col >= MAP_COLS) return 1;

    int tile = currentRoom->mapData[row][col];
    return tile == 1 || tile == 2; // 3(문)은 통과 가능
}

//인접 타일들을 사각형(AABB) 형태로 병합하여 렌더링 호출 횟수(Draw Call)를 줄이는 최적화 렌더링
void DrawMap(SDL_Renderer* renderer, SDL_Texture* mapBg, SDL_Texture* wallTex, SDL_Texture* borderTex)
{
    if (mapBg) SDL_RenderCopy(renderer, mapBg, NULL, NULL);

    bool processed[MAP_ROWS][MAP_COLS];
    memset(processed, 0, sizeof(processed));

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int tileType = currentRoom->mapData[r][c];
            if (tileType == 0 || processed[r][c]) continue;

            // 가로(오른쪽) 방향으로 연속된 동일 타일 너비 계산
            int w = 1;
            while (c + w < MAP_COLS && currentRoom->mapData[r][c + w] == tileType && !processed[r][c + w]) w++;

            // 세로(아래쪽) 방향으로 동일 너비만큼 타일이 연속되는지 확인하며 높이 확장
            int h = 1;
            while (r + h < MAP_ROWS) {
                int ok = 1;
                for (int k = 0; k < w; k++) {
                    if (currentRoom->mapData[r + h][c + k] != tileType || processed[r + h][c + k]) {
                        ok = 0; break;
                    }
                }
                if (!ok) break;
                h++;
            }

            // 병합 완료된 타일 영역 처리 마킹
            for (int dr = 0; dr < h; dr++) {
                for (int dc = 0; dc < w; dc++) {
                    processed[r + dr][c + dc] = true;
                }
            }

            // 병합된 단일 크기대로 렌더링 수행
            SDL_Rect rect = { c * TILE_SIZE, r * TILE_SIZE, w * TILE_SIZE, h * TILE_SIZE };
            SDL_Texture* tex = (tileType == 2) ? borderTex : wallTex;

            if (tex) {
                SDL_RenderCopy(renderer, tex, NULL, &rect);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

//모든 던전 노드의 변수 상태를 기본 상태로 리셋
void InitRoomNodes()
{
    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            roomNodes[x][y].gridX = x;
            roomNodes[x][y].gridY = y;
            roomNodes[x][y].exists = false;
            roomNodes[x][y].visited = false;
            roomNodes[x][y].discovered = false;
            roomNodes[x][y].specialCleared = false;

            roomNodes[x][y].roomType = ROOM_NORMAL;
        }
    }
}

//시작방 설정, 원거리 보스방 생성, 경로 확보, 랜덤 분기 생성, 특수방 배치 및 포인터 링크 연결
void GenerateDungeon()
{
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    int startX = MAX_ROOMS_X / 2;
    int startY = MAX_ROOMS_Y / 2;
    currentRoomX = startX;
    currentRoomY = startY;

    roomNodes[startX][startY].exists = true;
    roomNodes[startX][startY].roomType = ROOM_START;

    // 보스방 생성: 시작 위치에서 맨해튼 거리가 최소 6 이상 떨어질 때까지 외곽 가장자리에 배치
    int bossX = 0, bossY = 0, dist = 0;
    do {
        int side = rand() % 4;
        switch (side) {
        case 0: bossX = rand() % MAX_ROOMS_X; bossY = 0; break;
        case 1: bossX = rand() % MAX_ROOMS_X; bossY = MAX_ROOMS_Y - 1; break;
        case 2: bossX = 0; bossY = rand() % MAX_ROOMS_Y; break;
        case 3: bossX = MAX_ROOMS_X - 1; bossY = rand() % MAX_ROOMS_Y; break;
        }
        dist = abs(bossX - startX) + abs(bossY - startY);
    } while (dist < 6);

    roomNodes[bossX][bossY].exists = true;
    roomNodes[bossX][bossY].roomType = ROOM_BOSS;
    printf("Boss Room: (%d, %d)\n", bossX, bossY);

    // 시작방에서 보스방까지 끊기지 않는 메인 스트레이트 경로 생성
    int x = startX, y = startY;
    while (x != bossX || y != bossY) {
        int dir = rand() % 2;
        if (x != bossX && (y == bossY || dir == 0)) {
            x += (x < bossX) ? 1 : -1;
        }
        else if (y != bossY) {
            y += (y < bossY) ? 1 : -1;
        }
        roomNodes[x][y].exists = true;

        // 주 경로 주변에 랜덤하게 인접 방 하나씩 곁가지 생성
        if (rand() % 3 == 0) {
            roomNodes[x + rand() % 2 - 1][y].exists = true;
        }
    }

    // 메인 경로 외에 던전 볼륨을 늘리기 위한 50개의 추가 서브 랜덤 가지(Branch) 확충
    for (int i = 0; i < 50; i++) {
        int rx = rand() % MAX_ROOMS_X;
        int ry = rand() % MAX_ROOMS_Y;
        if (!roomNodes[rx][ry].exists) continue;

        int branchLength = 1 + rand() % 3;
        int dir = rand() % 4;
        int nx = rx, ny = ry;

        for (int j = 0; j < branchLength; j++) {
            switch (dir) {
            case 0: ny--; break;
            case 1: ny++; break;
            case 2: nx--; break;
            case 3: nx++; break;
            }
            if (nx < 0 || nx >= MAX_ROOMS_X || ny < 0 || ny >= MAX_ROOMS_Y) break;
            roomNodes[nx][ny].exists = true;
        }
    }

    int mazeX, mazeY;
    do {
        mazeX = rand() % MAX_ROOMS_X; mazeY = rand() % MAX_ROOMS_Y;
    } while (!roomNodes[mazeX][mazeY].exists || roomNodes[mazeX][mazeY].roomType != ROOM_NORMAL);

    // 실제 설정 개수만큼 미로 스페셜 방 할당
    for (int i = 0; i < MAZE_ROOM_COUNT; i++) {
        int mazeX, mazeY, tryCount = 0;
        do {
            mazeX = rand() % MAX_ROOMS_X; mazeY = rand() % MAX_ROOMS_Y;
            tryCount++;
        } while ((!roomNodes[mazeX][mazeY].exists || roomNodes[mazeX][mazeY].roomType != ROOM_NORMAL) && tryCount < 1000);

        if (tryCount < 1000) roomNodes[mazeX][mazeY].roomType = ROOM_MAZE;
    }

    int quizX, quizY;
    do {
        quizX = rand() % MAX_ROOMS_X; quizY = rand() % MAX_ROOMS_Y;
    } while (!roomNodes[quizX][quizY].exists || roomNodes[quizX][quizY].roomType != ROOM_NORMAL);

    // 실제 설정 개수만큼 퀴즈 스페셜 방 할당
    for (int i = 0; i < QUIZ_ROOM_COUNT; i++) {
        int mazeX, mazeY, tryCount = 0; // 변수명이 mazeX로 되어있으나 구조상 작동은 함
        do {
            mazeX = rand() % MAX_ROOMS_X; mazeY = rand() % MAX_ROOMS_Y;
            tryCount++;
        } while ((!roomNodes[mazeX][mazeY].exists || roomNodes[mazeX][mazeY].roomType != ROOM_NORMAL) && tryCount < 1000);

        if (tryCount < 1000) roomNodes[mazeX][mazeY].roomType = ROOM_QUIZ;
    }

     // 상하좌우 노드 간의 포인터 링크 자동 연결
    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            roomNodes[x][y].up = NULL;
            roomNodes[x][y].down = NULL;
            roomNodes[x][y].left = NULL;
            roomNodes[x][y].right = NULL;

            if (!roomNodes[x][y].exists) continue;

            if (y > 0 && roomNodes[x][y - 1].exists) roomNodes[x][y].up = &roomNodes[x][y - 1];
            if (y < MAX_ROOMS_Y - 1 && roomNodes[x][y + 1].exists) roomNodes[x][y].down = &roomNodes[x][y + 1];
            if (x > 0 && roomNodes[x - 1][y].exists) roomNodes[x][y].left = &roomNodes[x - 1][y];
            if (x < MAX_ROOMS_X - 1 && roomNodes[x + 1][y].exists) roomNodes[x][y].right = &roomNodes[x + 1][y];
        }
    }
    currentRoom = &roomNodes[startX][startY];
}

//다른 맵으로 전환될 때 기존 방의 데이터 저장, 다음 방의 몬스터 데이터 로드 및 미로/퀴즈 초기화 트리거
void MoveToNextRoom(int direction)
{
    SaveEnemiesForRoom(currentRoomX, currentRoomY);

    switch (direction) {
    case 0: if (currentRoom->up && currentRoom->up->exists) currentRoom = currentRoom->up; break;
    case 1: if (currentRoom->down && currentRoom->down->exists) currentRoom = currentRoom->down; break;
    case 2: if (currentRoom->left && currentRoom->left->exists) currentRoom = currentRoom->left; break;
    case 3: if (currentRoom->right && currentRoom->right->exists) currentRoom = currentRoom->right; break;
    }

    currentRoomX = currentRoom->gridX;
    currentRoomY = currentRoom->gridY;

    LoadEnemiesForRoom(currentRoomX, currentRoomY);
    InitProjectiles();
    InitMap();
    currentRoom->visited = true;

    // 만약 이동한 곳이 클리어하지 않은 퀴즈방이라면 퀴즈 스테이지 즉시 개시
    if (currentRoom->roomType == ROOM_QUIZ && !currentRoom->specialCleared) {
        StartQuizStage();
        gGameState = GAME_QUIZ;
    }
}

// M키를 눌렀을 때 화면 전체를 덮어 전체 방들의 연결 상태와 종류를 보여주는 전체 지도 오버레이 렌더링
void DrawMapOverlay(SDL_Renderer* renderer)
{
    if (!gShowFullMap) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect fullScreen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &fullScreen);

    int roomSize = 40;
    int padding = 5;
    int offsetX = (SCREEN_WIDTH - (MAX_ROOMS_X * (roomSize + padding))) / 2;
    int offsetY = (SCREEN_HEIGHT - (MAX_ROOMS_Y * (roomSize + padding))) / 2;

    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            if (!roomNodes[x][y].exists) continue;

            SDL_Rect roomRect = { offsetX + x * (roomSize + padding), offsetY + y * (roomSize + padding), roomSize, roomSize };
            RoomNode* room = &roomNodes[x][y];

            if (x == currentRoomX && y == currentRoomY) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 현재 방
            else if (room->roomType == ROOM_BOSS) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);       // 보스방
            else if (room->roomType == ROOM_MAZE) SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);     // 미로방
            else if (room->roomType == ROOM_QUIZ) SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255);     // 퀴즈방
            else if (room->visited) SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);                 // 탐색 완료
            else SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);                                       // 미탐색

            SDL_RenderFillRect(renderer, &roomRect);
        }
    }
}

//화면 우측 상단에 현재 위치를 포함한 인접 3x3 범위의 국소 지도를 상시 보여주는 미니맵 UI 렌더링
void DrawMiniMap(SDL_Renderer* renderer)
{
    int miniMapX = SCREEN_WIDTH - 80;
    int miniMapY = 20;
    int roomSize = 15;
    int padding = 5;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect bgRect = { miniMapX - 5, miniMapY - 5, 65, 65 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &bgRect);

    int centerX = currentRoomX;
    int centerY = currentRoomY;

    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            if (!roomNodes[x][y].exists) continue;

            int dx = x - centerX;
            int dy = y - centerY;

            // 현재 위치 기준 반경 1칸 이내(3x3)만 스크리닝하여 드로우 대상 압축
            if (abs(dx) > 1 || abs(dy) > 1) continue;

            int drawX = miniMapX + (dx + 1) * (roomSize + padding);
            int drawY = miniMapY + (dy + 1) * (roomSize + padding);
            SDL_Rect roomRect = { drawX, drawY, roomSize, roomSize };
            RoomNode* room = &roomNodes[x][y];

            if (x == currentRoomX && y == currentRoomY) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            else if (room->roomType == ROOM_BOSS) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else if (room->roomType == ROOM_MAZE) SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
            else if (room->roomType == ROOM_QUIZ) SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255);
            else if (room->visited) SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            else SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);

            SDL_RenderFillRect(renderer, &roomRect);
        }
    }
}
