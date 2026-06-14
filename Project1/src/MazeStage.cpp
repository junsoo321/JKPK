#include "MazeStage.hpp"
#include "Constants.h"

#include <SDL.h>
#include <vector>
#include <algorithm>
#include <queue>

MazeStageData gMazeStage;
float cameraX;
float cameraY;

static bool IsMazeWall(float x, float y)
{
    int col = (int)x / MAZE_TILE_SIZE;
    int row = (int)y / MAZE_TILE_SIZE;

    if (row < 0 || row >= MAZE_HEIGHT)
        return true;

    if (col < 0 || col >= MAZE_WIDTH)
        return true;

    return gMazeStage.maze[row][col] == 1;
}

static void CarveMaze(int x, int y)
{
    static const int dx[4] = { 0, 0, -2, 2 };
    static const int dy[4] = { -2, 2, 0, 0 };

    std::vector<int> dirs = { 0,1,2,3 };

    std::random_shuffle(
        dirs.begin(),
        dirs.end()
    );

    for (int dir : dirs)
    {
        int nx = x + dx[dir];
        int ny = y + dy[dir];

        if (
            nx <= 0 ||
            ny <= 0 ||
            nx >= MAZE_WIDTH - 1 ||
            ny >= MAZE_HEIGHT - 1
            )
        {
            continue;
        }

        if (gMazeStage.maze[ny][nx] == 0)
        {
            continue;
        }

        gMazeStage.maze[ny][nx] = 0;

        gMazeStage.maze[
            y + dy[dir] / 2
        ][
            x + dx[dir] / 2
        ] = 0;

        CarveMaze(
            nx,
            ny
        );
    }
}

void StartMazeStage()
{
    gMazeStage.finished = false;
    gMazeStage.keyCollected = false;

    for (int r = 0; r < MAZE_HEIGHT; r++)
    {
        for (int c = 0; c < MAZE_WIDTH; c++)
        {
            gMazeStage.maze[r][c] = 1;
        }
    }

    gMazeStage.playerX = MAZE_TILE_SIZE + 2;
    gMazeStage.playerY = MAZE_TILE_SIZE + 2;

    gMazeStage.maze[1][1] = 0;

    CarveMaze(
        1,
        1
    );

    int keyCellX;
    int keyCellY;

    FindFarthestPoint(
        1,
        1,
        keyCellX,
        keyCellY
    );

    gMazeStage.keyX = keyCellX;
    gMazeStage.keyY = keyCellY;
}

void UpdateMazeStage(
    const Uint8* keyState,
    float deltaTime
)
{
    float move = 200.0f * deltaTime;

    float nextX = gMazeStage.playerX;
    float nextY = gMazeStage.playerY;

    if (keyState[SDL_SCANCODE_W])
        nextY -= move;

    if (keyState[SDL_SCANCODE_S])
        nextY += move;

    if (keyState[SDL_SCANCODE_A])
        nextX -= move;

    if (keyState[SDL_SCANCODE_D])
        nextX += move;

    bool blocked = false;

    if (IsMazeWall(nextX, nextY))
        blocked = true;

    if (IsMazeWall(nextX + MAZE_PLAYER_SIZE - 1, nextY))
        blocked = true;

    if (IsMazeWall(nextX, nextY + MAZE_PLAYER_SIZE - 1))
        blocked = true;

    if (IsMazeWall(
        nextX + MAZE_PLAYER_SIZE - 1,
        nextY + MAZE_PLAYER_SIZE - 1))
    {
        blocked = true;
    }

    if (!blocked)
    {
        gMazeStage.playerX = nextX;
        gMazeStage.playerY = nextY;
    }

    SDL_Rect playerRect =
    {
        (int)gMazeStage.playerX,
        (int)gMazeStage.playerY,
        MAZE_PLAYER_SIZE,
        MAZE_PLAYER_SIZE
    };

    SDL_Rect keyRect =
    {
        gMazeStage.keyX * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 4,
        gMazeStage.keyY * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 4,
        MAZE_TILE_SIZE / 2,
        MAZE_TILE_SIZE / 2
    };

    if (SDL_HasIntersection(
        &playerRect,
        &keyRect))
    {
        gMazeStage.keyCollected = true;
        gMazeStage.finished = true;
    }
}

void DrawMazeStage(SDL_Renderer* renderer)
{
    cameraX =
        gMazeStage.playerX +
        MAZE_PLAYER_SIZE / 2.0f -
        SCREEN_WIDTH / 2.0f;

    cameraY =
        gMazeStage.playerY +
        MAZE_PLAYER_SIZE / 2.0f -
        SCREEN_HEIGHT / 2.0f;

    SDL_SetRenderDrawColor(
        renderer,
        20,
        20,
        20,
        255
    );

    SDL_RenderClear(renderer);

    // 벽
    for (int r = 0; r < MAZE_HEIGHT; r++)
    {
        for (int c = 0; c < MAZE_WIDTH; c++)
        {
            if (gMazeStage.maze[r][c] == 0)
                continue;

            SDL_Rect wall =
            {
                (int)(c * MAZE_TILE_SIZE - cameraX),
                (int)(r * MAZE_TILE_SIZE - cameraY),
                MAZE_TILE_SIZE,
                MAZE_TILE_SIZE
            };

            SDL_SetRenderDrawColor(
                renderer,
                80,
                120,
                255,
                255
            );

            SDL_RenderFillRect(
                renderer,
                &wall
            );
        }
    }

    // 열쇠
    SDL_Rect keyRect =
    {
        (int)(gMazeStage.keyX * MAZE_TILE_SIZE - cameraX),
        (int)(gMazeStage.keyY * MAZE_TILE_SIZE - cameraY),
        MAZE_TILE_SIZE,
        MAZE_TILE_SIZE
    };

    SDL_SetRenderDrawColor(
        renderer,
        255,
        215,
        0,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &keyRect
    );

    // 플레이어
    SDL_Rect playerRect =
    {
        SCREEN_WIDTH / 2 - MAZE_PLAYER_SIZE / 2,
        SCREEN_HEIGHT / 2 - MAZE_PLAYER_SIZE / 2,
        MAZE_PLAYER_SIZE,
        MAZE_PLAYER_SIZE
    };

    SDL_SetRenderDrawColor(
        renderer,
        0,
        255,
        0,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &playerRect
    );
}

bool IsMazeFinished()
{
    return gMazeStage.finished;
}

static void FindFarthestPoint(
    int startX,
    int startY,
    int& outX,
    int& outY
)
{
    std::queue<std::pair<int, int>> q;

    static int dist[
        MAZE_HEIGHT
    ][
        MAZE_WIDTH
    ];

    for (int r = 0; r < MAZE_HEIGHT; r++)
    {
        for (int c = 0; c < MAZE_WIDTH; c++)
        {
            dist[r][c] = -1;
        }
    }

    q.push({ startX,startY });

    dist[startY][startX] = 0;

    outX = startX;
    outY = startY;

    static const int dx[4] = { 1,-1,0,0 };
    static const int dy[4] = { 0,0,1,-1 };

    while (!q.empty())
    {
        auto current = q.front();
        q.pop();

        int x = current.first;
        int y = current.second;

        if (
            dist[y][x] >
            dist[outY][outX]
            )
        {
            outX = x;
            outY = y;
        }

        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (
                nx < 0 ||
                ny < 0 ||
                nx >= MAZE_WIDTH ||
                ny >= MAZE_HEIGHT
                )
            {
                continue;
            }

            if (
                gMazeStage.maze[ny][nx] == 1
                )
            {
                continue;
            }

            if (
                dist[ny][nx] != -1
                )
            {
                continue;
            }

            dist[ny][nx] =
                dist[y][x] + 1;

            q.push({
                nx,
                ny
                });
        }
    }
}
