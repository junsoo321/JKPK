#pragma once
#include "Player.hpp"

// 새 아이템 추가 시: 1) 이 열거형에 항목 추가  2) Item.cpp ApplyItem/GetItemAssetName 케이스 추가
//                    3) assets/player/item/ 에 PNG 에셋 추가  4) ImageManager.cpp에서 자동 로드됨
enum ItemType {
    ITEM_MONSTER_ULTRA_ENERGY = 0,  // 이동속도 +30%
    ITEM_MECHANICAL_KEYBOARD,       // 투사체 속도 +20%
    ITEM_TRIANGLE_GIMBAP,           // 소모성 — 최대 체력의 절반 회복
    ITEM_COUNT
};

#define ITEM_NONE (-1)

// 소모성 아이템 여부 (인벤토리에 기록하지 않음)
bool IsConsumableItem(int itemType);

// 아이템 효과 적용 + 인벤토리 기록 (소모성 제외)
void ApplyItem(PlayerData* player, int itemType);

// 아직 획득하지 않은 아이템 중 랜덤 선택 (없으면 ITEM_NONE)
int PickUncollectedItem(const PlayerData* player);

// 에셋 파일명 반환 (assets/player/item/ 상대 경로)
const char* GetItemAssetName(int itemType);

// 아이템 표시 이름
const char* GetItemName(int itemType);

// 아이템 효과 설명
const char* GetItemDescription(int itemType);
