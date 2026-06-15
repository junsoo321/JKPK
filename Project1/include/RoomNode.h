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
    int layoutVariant; // 0=기본, 1=A, 2=B, 3=C (일반 방 전용 장애물 변형)

    int gridX;
    int gridY;

    struct RoomNode* up;
    struct RoomNode* down;
    struct RoomNode* left;
    struct RoomNode* right;

} RoomNode;
