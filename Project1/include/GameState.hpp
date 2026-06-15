#pragma once

enum GameState
{
    GAME_TITLE,
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
    GAME_OVER
};

extern GameState gGameState;
