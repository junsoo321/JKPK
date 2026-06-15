#pragma execution_character_set("utf-8")
#include "Item.hpp"
#include "Constants.h"

void ApplyItem(PlayerData* player, int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY:
        player->speed *= 1.3f;
        break;
    default:
        break;
    }
    if (player->itemCount < MAX_INVENTORY)
        player->collectedItems[player->itemCount++] = itemType;
}

const char* GetItemAssetName(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "monster-ultra-energy.png";
    default:                        return nullptr;
    }
}

const char* GetItemName(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "energy drink";
    default:                        return "???";
    }
}

const char* GetItemDescription(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "faster, faster - !";
    default:                        return "";
    }
}
