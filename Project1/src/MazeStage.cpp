#include "MazeStage.hpp"
#include "ImageManager.hpp"
#include "Constants.h"
#include <SDL.h>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>

MazeStageData gMazeStage;
float cameraX;
float cameraY;

//해당 좌표가 미로의 벽 영역(1)인지 확인하는 함수
static bool IsMazeWall(float x, float y)
{
    int col = (int)x / MAZE_TILE_SIZE;
    int row = (int)y / MAZE_TILE_SIZE;

    if (row < 0 || row >= MAZE_HEIGHT) return true;
    if (col < 0 || col >= MAZE_WIDTH) return true;

    return gMazeStage.maze[row][col] == 1;
}

//DFS(깊이 우선 탐색) 기반으로 2칸씩 뛰며 맵을 무작위로 파내는 재귀 미로 생성 함수
static void CarveMaze(int x, int y)
{
    static const int dx[4] = { 0, 0, -2, 2 };
    static const int dy[4] = { -2, 2, 0, 0 };

    std::vector<int> dirs = { 0, 1, 2, 3 };
    std::random_shuffle(dirs.begin(), dirs.end());

    for (int dir : dirs) {
        int nx = x + dx[dir];
        int ny = y + dy[dir];

        if (nx <= 0 || ny <= 0 || nx >= MAZE_WIDTH - 1 || ny >= MAZE_HEIGHT - 1) continue;
        if (gMazeStage.maze[ny][nx] == 0) continue;

        gMazeStage.maze[ny][nx] = 0;
        gMazeStage.maze[y + dy[dir] / 2][x + dx[dir] / 2] = 0; // 목적지와 현재 위치 사이의 벽 허물기

        CarveMaze(nx, ny);
    }
}

//미로 데이터 청소, 플레이어 시작 위치 지정, DFS 생성 트리거 및 가장 먼 곳에 열쇠 배치
void StartMazeStage()
{
    gMazeStage.finished = false;
    gMazeStage.keyCollected = false;

    for (int r = 0; r < MAZE_HEIGHT; r++) {
        for (int c = 0; c < MAZE_WIDTH; c++) {
            gMazeStage.maze[r][c] = 1;
        }
    }

    gMazeStage.playerX = MAZE_TILE_SIZE + 2;
    gMazeStage.playerY = MAZE_TILE_SIZE + 2;
    gMazeStage.maze[1][1] = 0;

    CarveMaze(1, 1);
    BreakRandomWalls(200); //완전 가둬진 트리 구조 대신 탈출구 다변화를 위해 벽 무작위 파괴

    int keyCellX, keyCellY;
    FindFarthestPoint(1, 1, keyCellX, keyCellY);

    gMazeStage.keyX = keyCellX;
    gMazeStage.keyY = keyCellY;
}

//플레이어 이동 처리, 4방향 AABB 벽 충돌 체크 및 열쇠 콜라이더 트리거 판정
void UpdateMazeStage(const Uint8* keyState, float deltaTime)
{
    float move = 200.0f * deltaTime;
    float nextX = gMazeStage.playerX;
    float nextY = gMazeStage.playerY;

    if (keyState[SDL_SCANCODE_W]) nextY -= move;
    if (keyState[SDL_SCANCODE_S]) nextY += move;
    if (keyState[SDL_SCANCODE_A]) nextX -= move;
    if (keyState[SDL_SCANCODE_D]) nextX += move;

    // 플레이어 캐릭터의 4개 꼭짓점(Box의 모서리들)을 모두 체크하여 완벽한 벽 충돌 구현
    bool blocked = false;
    if (IsMazeWall(nextX, nextY)) blocked = true;
    if (IsMazeWall(nextX + MAZE_PLAYER_SIZE - 1, nextY)) blocked = true;
    if (IsMazeWall(nextX, nextY + MAZE_PLAYER_SIZE - 1)) blocked = true;
    if (IsMazeWall(nextX + MAZE_PLAYER_SIZE - 1, nextY + MAZE_PLAYER_SIZE - 1)) blocked = true;

    if (!blocked) {
        gMazeStage.playerX = nextX;
        gMazeStage.playerY = nextY;
    }

    SDL_Rect playerRect = { (int)gMazeStage.playerX, (int)gMazeStage.playerY, MAZE_PLAYER_SIZE, MAZE_PLAYER_SIZE };
    SDL_Rect keyRect = { gMazeStage.keyX * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 4, gMazeStage.keyY * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 4, MAZE_TILE_SIZE / 2, MAZE_TILE_SIZE / 2 };

    if (SDL_HasIntersection(&playerRect, &keyRect)) {
        gMazeStage.keyCollected = true;
        gMazeStage.finished = true;
    }
}

//플레이어 상대 좌표 기준으로 월드 카메라 뷰포트 오프셋 계산 및 맵/열쇠/플레이어/나침반 렌더링
void DrawMazeStage(SDL_Renderer* renderer)
{
    // 플레이어가 월드 중앙에 오도록 카메라 스크롤 값 역산
    cameraX = gMazeStage.playerX + MAZE_PLAYER_SIZE / 2.0f - SCREEN_WIDTH / 2.0f;
    cameraY = gMazeStage.playerY + MAZE_PLAYER_SIZE / 2.0f - SCREEN_HEIGHT / 2.0f;

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    // 카메라 기준 화면에 보이는 타일 범위만 계산
    int colStart = (int)(cameraX / MAZE_TILE_SIZE) - 1;
    int colEnd   = colStart + (SCREEN_WIDTH  / MAZE_TILE_SIZE) + 2;
    int rowStart = (int)(cameraY / MAZE_TILE_SIZE) - 1;
    int rowEnd   = rowStart + (SCREEN_HEIGHT / MAZE_TILE_SIZE) + 2;

    if (colStart < 0) colStart = 0;
    if (rowStart < 0) rowStart = 0;
    if (colEnd   > MAZE_WIDTH)  colEnd  = MAZE_WIDTH;
    if (rowEnd   > MAZE_HEIGHT) rowEnd  = MAZE_HEIGHT;

    //카메라 오프셋이 적용된 스크롤 미로 타일 출력
    for (int r = rowStart; r < rowEnd; r++) {
        for (int c = colStart; c < colEnd; c++) {
            SDL_Rect tile = { (int)(c * MAZE_TILE_SIZE - cameraX), (int)(r * MAZE_TILE_SIZE - cameraY), MAZE_TILE_SIZE, MAZE_TILE_SIZE };

            if (gMazeStage.maze[r][c] == 1) {
                if (gMazeWallTex) {
                    SDL_RenderCopy(renderer, gMazeWallTex, NULL, &tile);
                } else {
                    SDL_SetRenderDrawColor(renderer, 80, 120, 255, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
            } else {
                if (gMazeFloorTex) {
                    SDL_RenderCopy(renderer, gMazeFloorTex, NULL, &tile);
                }
            }
        }
    }

    //스크롤 반영된 열쇠 목표물 렌더링
    SDL_Rect keyRect = { (int)(gMazeStage.keyX * MAZE_TILE_SIZE - cameraX), (int)(gMazeStage.keyY * MAZE_TILE_SIZE - cameraY), MAZE_TILE_SIZE, MAZE_TILE_SIZE };
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderFillRect(renderer, &keyRect);

    //플레이어 캐릭터 본체 (중앙 고정 뷰)
    SDL_Rect playerRect = { SCREEN_WIDTH / 2 - MAZE_PLAYER_SIZE / 2, SCREEN_HEIGHT / 2 - MAZE_PLAYER_SIZE / 2, MAZE_PLAYER_SIZE, MAZE_PLAYER_SIZE };
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &playerRect);

    //플레이어 위치에서 화면 밖 열쇠까지의 삼각함수(atan2f)각도를 계산하여 상단에 가이드 화살표 드로우
    float keyWorldX = gMazeStage.keyX * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 2.0f;
    float keyWorldY = gMazeStage.keyY * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 2.0f;
    float playerCenterX = gMazeStage.playerX + MAZE_PLAYER_SIZE / 2.0f;
    float playerCenterY = gMazeStage.playerY + MAZE_PLAYER_SIZE / 2.0f;

    float dx = keyWorldX - playerCenterX;
    float dy = keyWorldY - playerCenterY;
    float angle = atan2f(dy, dx);

    float arrowX = SCREEN_WIDTH / 2.0f;
    float arrowY = SCREEN_HEIGHT / 2.0f - 40.0f; // 캐릭터 바로 머리 위 배치
    const float size = 12.0f;

    SDL_Point tri[4];
    tri[0].x = (int)(arrowX + cosf(angle) * size);
    tri[0].y = (int)(arrowY + sinf(angle) * size);
    tri[1].x = (int)(arrowX + cosf(angle + 2.5f) * size);
    tri[1].y = (int)(arrowY + sinf(angle + 2.5f) * size);
    tri[2].x = (int)(arrowX + cosf(angle - 2.5f) * size);
    tri[2].y = (int)(arrowY + sinf(angle - 2.5f) * size);
    tri[3] = tri[0]; // 폴리라인 폐쇄용 복사

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderDrawLines(renderer, tri, 4);
}

bool IsMazeFinished()
{
    return gMazeStage.finished;
}

 // BFS(너비 우선 탐색)를 활용해 시작 타일에서 최장 가중치(가장 먼 거리)를 가지는 타일을 탐색 (열쇠 드롭용)
static void FindFarthestPoint(int startX, int startY, int& outX, int& outY)
{
    std::queue<std::pair<int, int>> q;
    static int dist[MAZE_HEIGHT][MAZE_WIDTH];

    for (int r = 0; r < MAZE_HEIGHT; r++) {
        for (int c = 0; c < MAZE_WIDTH; c++) {
            dist[r][c] = -1;
        }
    }

    q.push({ startX, startY });
    dist[startY][startX] = 0;
    outX = startX;
    outY = startY;

    static const int dx[4] = { 1, -1, 0, 0 };
    static const int dy[4] = { 0, 0, 1, -1 };

    while (!q.empty()) {
        auto current = q.front();
        q.pop();

        int x = current.first;
        int y = current.second;

        // 역대 최대 깊이(거리)를 갱신하면 타겟 좌표를 해당 위치로 계속 갱신
        if (dist[y][x] > dist[outY][outX]) {
            outX = x;
            outY = y;
        }

        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || ny < 0 || nx >= MAZE_WIDTH || ny >= MAZE_HEIGHT) continue;
            if (gMazeStage.maze[ny][nx] == 1) continue;
            if (dist[ny][nx] != -1) continue;

            dist[ny][nx] = dist[y][x] + 1;
            q.push({ nx, ny });
        }
    }
}

 //완성된 격자 구조 벽을 무작위 개수만큼 뚫어 우회로 및 순환 루프 맵 생성
void BreakRandomWalls(int count)
{
    for (int i = 0; i < count; i++) {
        int x = rand() % (MAZE_WIDTH - 2) + 1;
        int y = rand() % (MAZE_HEIGHT - 2) + 1;

        if (gMazeStage.maze[y][x] == 0) continue;

        // 미로의 절대적인 외곽 테두리 경계벽은 파괴 대상에서 예외 처리하여 맵 이탈 방지
        if (x <= 1 || y <= 1 || x >= MAZE_WIDTH - 2 || y >= MAZE_HEIGHT - 2) continue;

        gMazeStage.maze[y][x] = 0;
    }
}
