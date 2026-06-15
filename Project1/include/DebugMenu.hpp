#pragma once
#ifdef _DEBUG
#include <SDL.h>

enum DebugAction {
    DEBUG_ACTION_NONE = 0,
    DEBUG_ACTION_CHEAT_INVINCIBLE,
    DEBUG_ACTION_CHEAT_DAMAGE_100X,
    DEBUG_ACTION_MAP_BOSS,
    DEBUG_ACTION_MAP_MAZE,
    DEBUG_ACTION_MAP_QUIZ,
    DEBUG_ACTION_MAP_PLAIN,
    DEBUG_ACTION_SPAWN_NORMAL,
    DEBUG_ACTION_SPAWN_NINJA,
    DEBUG_ACTION_SPAWN_SUICIDE
};

void        DebugMenuHandleEvent(SDL_Event& event);
void        DebugMenuRender(SDL_Renderer* renderer);
bool        DebugMenuIsOpen();
bool        DebugMenuIsMenuOpen();
bool        DebugMenuIsUnlocked();
DebugAction DebugMenuGetAction();

#endif
