#pragma once

#include <Constants.h>
#include <stdbool.h>

typedef struct RoomNode {

    int mapData[MAP_ROWS][MAP_COLS];

    bool visited;
    bool discovered;
    bool exists;
    bool specialCleared;
    bool rewardAvailable; // 탁자+아이템 표시 여부
    bool rewardCollected; // 보상을 이미 수령했는지 (재생성 방지)
    int  tableItemType;   // ITEM_NONE(-1) 또는 ItemType 값

    int roomType;
    int layoutVariant; // 0=기본, 1=A, 2=B, 3=C (일반 방 전용 장애물 변형)

    int gridX;
    int gridY;

    struct RoomNode* up;
    struct RoomNode* down;
    struct RoomNode* left;
    struct RoomNode* right;

} RoomNode;
