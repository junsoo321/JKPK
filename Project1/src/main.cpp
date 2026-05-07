#include <SDL.h>
#include "Constants.h"
#include "MapSystem.h"
#include "Player.hpp"
#include "ImageManager.hpp"
#include "Projectile.hpp"
#include "Enemy.h"

int main(int argc, char* argv[]) {
    //SDL 초기화 및 창 생성 코드
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Game Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //초기화
    InitMap();                 //맵 초기화
    LoadAllImages(renderer);   //이미지 매니저 초기화

    PlayerData player;         //플레이어 구조체 변수 선언
    InitPlayer(&player);       //초기값 할당 함수 호출

    bool isRunning = true;     //게임 실행 여부 플래그
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    FireProjectile(player.x, player.y);
                }
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);
        UpdatePlayer(&player, state);

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        UpdateAndDrawProjectiles(renderer);
        UpdateAndDrawEnemies(renderer, player.x, player.y);
        CheckEnemyCollision(bullets);

        DrawMap(renderer);
        DrawPlayer(renderer, &player);
        DrawMiniMap(renderer);

        SDL_RenderPresent(renderer);
    }

    return 0;
}
