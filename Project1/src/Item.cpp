#pragma execution_character_set("utf-8")
#include "Item.hpp"
#include "Constants.h"
#include <cstdlib>

bool IsConsumableItem(int itemType) {
    return itemType == ITEM_TRIANGLE_GIMBAP;
}

void ApplyItem(PlayerData* player, int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY:
        player->speed *= 1.3f;
        break;
    case ITEM_MECHANICAL_KEYBOARD:
        player->projectileSpeedMult *= 1.2f;
        break;
    case ITEM_TRIANGLE_GIMBAP: {
        int heal = PLAYER_HP / 2;
        player->hp += heal;
        if (player->hp > PLAYER_HP) player->hp = PLAYER_HP;
        player->healTimer = 0.6f;
        break;
    }
    default:
        break;
    }

    // 소모성 아이템은 인벤토리에 기록하지 않음
    if (!IsConsumableItem(itemType) && player->itemCount < MAX_INVENTORY)
        player->collectedItems[player->itemCount++] = itemType;
}

const char* GetItemAssetName(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "monster-ultra-energy.png";
    case ITEM_MECHANICAL_KEYBOARD:  return "mechanical-keyboard.png";
    case ITEM_TRIANGLE_GIMBAP:      return "triangle-gimbap.png";
    default:                        return nullptr;
    }
}

const char* GetItemName(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "energy drink";
    case ITEM_MECHANICAL_KEYBOARD:  return "기계식 키보드";
    case ITEM_TRIANGLE_GIMBAP:      return "삼각김밥";
    default:                        return "???";
    }
}

const char* GetItemDescription(int itemType) {
    switch ((ItemType)itemType) {
    case ITEM_MONSTER_ULTRA_ENERGY: return "faster, faster - !";
    case ITEM_MECHANICAL_KEYBOARD:  return "Sounds Noisy, But Feels GREAT - !";
    case ITEM_TRIANGLE_GIMBAP:      return "Bon Appetit !";
    default:                        return "";
    }
}

int PickUncollectedItem(const PlayerData* player) {
    int candidates[ITEM_COUNT];
    int count = 0;
    for (int i = 0; i < ITEM_COUNT; i++) {
        // 소모성 아이템은 항상 후보에 포함
        if (IsConsumableItem(i)) {
            candidates[count++] = i;
            continue;
        }
        // 영구 아이템은 미획득인 경우만 포함
        bool already = false;
        for (int j = 0; j < player->itemCount; j++) {
            if (player->collectedItems[j] == i) { already = true; break; }
        }
        if (!already) candidates[count++] = i;
    }
    if (count == 0) return ITEM_NONE;
    return candidates[rand() % count];
}
