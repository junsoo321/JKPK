#pragma once

#include <SDL.h>

#define MAZE_WIDTH 81
#define MAZE_HEIGHT 61

struct MazeStageData
{
    int maze[MAZE_HEIGHT][MAZE_WIDTH];

    float playerX;
    float playerY;

    int keyX;
    int keyY;

    bool keyCollected;
    bool finished;
};

extern MazeStageData gMazeStage;

void StartMazeStage();

void UpdateMazeStage(
    const Uint8* keyState,
    float deltaTime
);

void DrawMazeStage(
    SDL_Renderer* renderer
);

bool IsMazeFinished();

static void FindFarthestPoint(
    int startX,
    int startY,
    int& outX,
    int& outY
);

void BreakRandomWalls(int count);
