#include "MapSystem.hpp"
#include "MapData.hpp" 
#include "Constants.h"
#include "Projectile.hpp"
#include "Enemy.h"
#include "Boss.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

RoomNode roomNodes[MAX_ROOMS_X][MAX_ROOMS_Y];
extern BossData mainBoss;
extern bool isBossFight;
bool gShowFullMap = false;

//랜덤하게 맵 패턴을 가져와서 맵을 생성하는 함수
void InitMap() {
    currentRoom = &roomNodes[currentRoomX][currentRoomY];
    //이미 생성된 맵인지 확인(방문했던 맵인지 확인)
    if (currentRoom->visited) {
        return;
    }
    int randomIdx = rand() % MAX_PATTERNS;

    //맵 초기화 및 패턴 생성, 전체 월드의 테두리(벽) 생성 (여기선 tile값만 조정, 실제 출력은 아래 DrawMap에서 구현)
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            //기본 바닥 설정
            int tile = 0;

            //월드 테두리 생성 (2: 가장자리 벽)
            if (r == 0 && currentRoomY == 0) tile = 2;
            if (r == MAP_ROWS - 1 && currentRoomY == MAX_ROOMS_Y - 1) tile = 2;
            if (c == 0 && currentRoomX == 0) tile = 2;
            if (c == MAP_COLS - 1 && currentRoomX == MAX_ROOMS_X - 1) tile = 2;

            //맵 패턴 생성
            if (r > 0 && r < MAP_ROWS - 1 && c > 0 && c < MAP_COLS - 1) {
                tile = mapLayouts[1][r][c];
            }

            //결과물을 현재 맵(worldMap)과 월드 데이터(worldData)에 동시 저장
            currentRoom->mapData[r][c] = tile;
        }
    }

    currentRoom->visited = true;//방문 표시
}

//벽 충돌 체크 함수
int IsWall(float x, float y) {
    // 보스 살아있으면 맵 경계를 벽으로 처리
    if (isBossFight && mainBoss.hp > 0) {
        if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
            return 1;
        }
    }

    int col = (int)(x / TILE_SIZE);
    int row = (int)(y / TILE_SIZE);

    if (row < 0 || row >= MAP_ROWS) {
        return 1;
    }

    if (col < 0 || col >= MAP_COLS) {
        return 1;
    }

    return currentRoom->mapData[row][col] != 0;
}

//맵 그리기 함수
//위에서 만든 지도(worldMap)를 가지고 실제 그림 출력
void DrawMap(SDL_Renderer* renderer, SDL_Texture* mapBg, SDL_Texture* wallTex, SDL_Texture* borderTex) {
    // 배경 이미지 전체 화면에 렌더링
    if (mapBg) {
        SDL_RenderCopy(renderer, mapBg, NULL, NULL);
    }

    // 이미 렌더링한 타일 추적 (인접 타일을 하나의 이미지로 합치기 위함)
    bool processed[MAP_ROWS][MAP_COLS];
    memset(processed, 0, sizeof(processed));

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int tileType = currentRoom->mapData[r][c];
            if (tileType == 0 || processed[r][c]) continue;

            // 오른쪽으로 최대 너비 확장 (같은 타일 타입만)
            int w = 1;
            while (c + w < MAP_COLS && currentRoom->mapData[r][c + w] == tileType && !processed[r][c + w])
                w++;

            // 아래쪽으로 최대 높이 확장 (같은 너비·같은 타입의 벽 타일이 연속될 때만)
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

            // 사용한 타일 전부 처리 완료 표시
            for (int dr = 0; dr < h; dr++)
                for (int dc = 0; dc < w; dc++)
                    processed[r + dr][c + dc] = true;

            // 타일 타입에 따라 텍스처 선택 (1: 내부 장애물, 2: 가장자리 벽)
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

void InitRoomNodes() {
    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            roomNodes[x][y].gridX = x;
            roomNodes[x][y].gridY = y;

            roomNodes[x][y].exists = false;
            roomNodes[x][y].visited = false;
            roomNodes[x][y].discovered = false;

            roomNodes[x][y].roomType = ROOM_NORMAL;
        }
    }
}

void GenerateDungeon() {
    static bool seeded = false;

    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    int startX = MAX_ROOMS_X / 2;
    int startY = MAX_ROOMS_Y / 2;
    currentRoomX = startX;
    currentRoomY = startY;

    //시작방 생성
    roomNodes[startX][startY].exists = true;
    roomNodes[startX][startY].roomType = ROOM_START;

    //보스방 생성
    int bossX = 0;
    int bossY = 0;
    int dist = 0;

    do {
        int side = rand() % 4;
        switch (side) {
        case 0:
            bossX = rand() % MAX_ROOMS_X;
            bossY = 0;
            break;

        case 1:
            bossX = rand() % MAX_ROOMS_X;
            bossY = MAX_ROOMS_Y - 1;
            break;

        case 2:
            bossX = 0;
            bossY = rand() % MAX_ROOMS_Y;
            break;

        case 3:
            bossX = MAX_ROOMS_X - 1;
            bossY = rand() % MAX_ROOMS_Y;
            break;
        }

        dist = abs(bossX - startX) + abs(bossY - startY);

    } while (dist < 6);

    roomNodes[bossX][bossY].exists = true;
    roomNodes[bossX][bossY].roomType = ROOM_BOSS;
    printf("Boss Room: (%d, %d)\n", bossX, bossY); //디버깅용

    //시작 → 보스 경로 생성

    int x = startX;
    int y = startY;

    while (x != bossX || y != bossY)
    {
        int dir = rand() % 2;

        if (x != bossX && (y == bossY || dir == 0)) {
            x += (x < bossX) ? 1 : -1;
        }
        else if (y != bossY) {
            y += (y < bossY) ? 1 : -1;
        }
        roomNodes[x][y].exists = true;

        if (rand() % 3 == 0) {
            roomNodes[x + rand() % 2 - 1][y].exists = true;
        }

    }

    for (int i = 0; i < 20; i++)
    {
        int rx = rand() % MAX_ROOMS_X;
        int ry = rand() % MAX_ROOMS_Y;

        if (!roomNodes[rx][ry].exists) {
            continue;
        }

        int dir = rand() % 4;

        int nx = rx;
        int ny = ry;

        switch (dir) {
        case 0: ny--; break;
        case 1: ny++; break;
        case 2: nx--; break;
        case 3: nx++; break;
        }

        if (nx < 0 || nx >= MAX_ROOMS_X || ny < 0 || ny >= MAX_ROOMS_Y) {
            continue;
        }

        roomNodes[nx][ny].exists = true;
    }

    //링크 연결

    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            roomNodes[x][y].up = NULL;
            roomNodes[x][y].down = NULL;
            roomNodes[x][y].left = NULL;
            roomNodes[x][y].right = NULL;

            if (!roomNodes[x][y].exists) {
                continue;
            }

            if (y > 0 && roomNodes[x][y - 1].exists) {
                roomNodes[x][y].up = &roomNodes[x][y - 1];
            }

            if (y < MAX_ROOMS_Y - 1 && roomNodes[x][y + 1].exists) {
                roomNodes[x][y].down = &roomNodes[x][y + 1];
            }

            if (x > 0 && roomNodes[x - 1][y].exists) {
                roomNodes[x][y].left = &roomNodes[x - 1][y];
            }

            if (x < MAX_ROOMS_X - 1 && roomNodes[x + 1][y].exists) {
                roomNodes[x][y].right = &roomNodes[x + 1][y];
            }
        }
    }
    currentRoom = &roomNodes[startX][startY];
}

//시작 방 위치 (중앙)
int currentRoomX = 2;
int currentRoomY = 2;
RoomNode* currentRoom = NULL;

//맵 끝까지 도달했을 경우, 다음 맵으로 이동하는 함수
void MoveToNextRoom(int direction) {

    //현재 맵의 몹 데이터 저장
    SaveEnemiesForRoom(currentRoomX, currentRoomY);

    //맵 좌표 변경
    switch (direction) {
    case 0:
        if (currentRoom->up && currentRoom->up->exists)
            currentRoom = currentRoom->up;
        break;

    case 1:
        if (currentRoom->down && currentRoom->down->exists)
            currentRoom = currentRoom->down;
        break;

    case 2:
        if (currentRoom->left && currentRoom->left->exists)
            currentRoom = currentRoom->left;
        break;

    case 3:
        if (currentRoom->right && currentRoom->right->exists)
            currentRoom = currentRoom->right;
        break;
    }

    currentRoomX = currentRoom->gridX;
    currentRoomY = currentRoom->gridY;

    LoadEnemiesForRoom(currentRoomX, currentRoomY); //다음 방의 몹 로드
    InitProjectiles(); //이전 방의 투사체 제거
    InitMap(); //다음 맵 생성(또는 불러오기)
    currentRoom->visited = true;
}

void DrawMapOverlay(SDL_Renderer* renderer)
{
    if (!gShowFullMap) return;

    //반투명 배경 (전체 화면 덮기)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);

    SDL_Rect fullScreen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &fullScreen);

    //맵 시작 위치 (화면 중앙 배치)
    int roomSize = 40;
    int padding = 5;

    int offsetX = (SCREEN_WIDTH - (MAX_ROOMS_X * (roomSize + padding))) / 2;
    int offsetY = (SCREEN_HEIGHT - (MAX_ROOMS_Y * (roomSize + padding))) / 2;

    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            if (!roomNodes[x][y].exists) {
                continue;
            }

            SDL_Rect roomRect = {offsetX + x * (roomSize + padding), offsetY + y * (roomSize + padding), roomSize, roomSize};
            RoomNode* room = &roomNodes[x][y];

            if (x == currentRoomX && y == currentRoomY) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            }
            else if (room->roomType == ROOM_BOSS) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
            else if (room->visited) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            }

            SDL_RenderFillRect(renderer, &roomRect);
        }
    }
}

//미니맵 그리는 함수
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
            if (!roomNodes[x][y].exists) {
                continue;
            }
            int dx = x - centerX;
            int dy = y - centerY;

            if (abs(dx) > 1 || abs(dy) > 1) {
                continue;
            }

            int drawX = miniMapX + (dx + 1) * (roomSize + padding);
            int drawY = miniMapY + (dy + 1) * (roomSize + padding);
            SDL_Rect roomRect = {drawX, drawY, roomSize, roomSize};
            RoomNode* room = &roomNodes[x][y];

            //미니맵 구분용 색상
            if (x == currentRoomX && y == currentRoomY) { //현재 위치한 방
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            }
            else if (room->roomType == ROOM_BOSS) { //보스 방
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
            else if (room->visited) { //방문한 방
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            else{ //미방문한 방
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            }

            SDL_RenderFillRect(renderer, &roomRect);
        }
    }
}
