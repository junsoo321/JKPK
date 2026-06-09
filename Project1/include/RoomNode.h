#pragma once

#include <Constants.h>
#include <stdbool.h>

typedef struct RoomNode {

    int mapData[MAP_ROWS][MAP_COLS];

    bool visited;
    bool discovered;
    bool exists;
    bool specialCleared;

    int roomType;

    int gridX;
    int gridY;

    struct RoomNode* up;
    struct RoomNode* down;
    struct RoomNode* left;
    struct RoomNode* right;

} RoomNode;
