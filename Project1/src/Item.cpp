#pragma execution_character_set("utf-8")
#include "Item.hpp"
#include "Constants.h"
#include <cstdlib>

void ApplyItem(PlayerData* player, int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY:
        player->speed *= 1.3f;
        break;
    case ITEM_MECHANICAL_KEYBOARD:
        player->projectileSpeedMult *= 1.2f;
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
    case ITEM_MECHANICAL_KEYBOARD:  return "mechanical-keyboard.png";
    default:                        return nullptr;
    }
}

const char* GetItemName(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "energy drink";
    case ITEM_MECHANICAL_KEYBOARD:  return "기계식 키보드";
    default:                        return "???";
    }
}

int PickUncollectedItem(const PlayerData* player) {
    int candidates[ITEM_COUNT];
    int count = 0;
    for (int i = 0; i < ITEM_COUNT; i++) {
        bool already = false;
        for (int j = 0; j < player->itemCount; j++) {
            if (player->collectedItems[j] == i) { already = true; break; }
        }
        if (!already) candidates[count++] = i;
    }
    if (count == 0) return ITEM_NONE;
    return candidates[rand() % count];
}

const char* GetItemDescription(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "faster, faster - !";
    case ITEM_MECHANICAL_KEYBOARD:  return "Sounds Noisy, But Feels GREAT - !";
    default:                        return "";
    }
}
