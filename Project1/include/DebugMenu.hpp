#pragma once
#ifdef _DEBUG
#include <SDL.h>

enum DebugAction {
    DEBUG_ACTION_NONE = 0,
    DEBUG_ACTION_MAP_BOSS,
    DEBUG_ACTION_MAP_MAZE,
    DEBUG_ACTION_MAP_QUIZ,
    DEBUG_ACTION_MAP_PLAIN
};

void        DebugMenuHandleEvent(SDL_Event& event);
void        DebugMenuRender(SDL_Renderer* renderer);
bool        DebugMenuIsOpen();
bool        DebugMenuIsMenuOpen();
bool        DebugMenuIsUnlocked();
DebugAction DebugMenuGetAction();

#endif
