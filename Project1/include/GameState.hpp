#pragma once

enum GameState
{
    GAME_TITLE,
    GAME_SETTINGS,
    GAME_STORY,
    GAME_HELP,

    GAME_NORMAL,
    GAME_MAZE_TRANSITION,
    GAME_MAZE,
    GAME_QUIZ_PROMPT,
    GAME_QUIZ,
    GAME_BOSS,

    GAME_PAUSE,

    GAME_OVER_FADE,
    GAME_OVER,

    GAME_CLEAR
};

extern GameState gGameState;

// 치트 플래그 (디버그 전용)
extern bool gCheatGodMode;
extern bool gCheatDamage100x;
