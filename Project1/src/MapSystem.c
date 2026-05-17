#include "MapSystem.h"
#include "MapData.h" 
#include "Constants.h"
#include "Projectile.hpp"
#include "Enemy.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

int worldMap[MAP_ROWS][MAP_COLS]; //현재 맵의 상태 데이터

int worldData[MAX_ROOMS_X][MAX_ROOMS_Y][MAP_ROWS][MAP_COLS]; //전체 월드의 상태 데이터
bool visited[MAX_ROOMS_X][MAX_ROOMS_Y] = { false }; //이미 생성된(방문했던) 맵인지 확인하는 배열

//랜덤하게 맵 패턴을 가져와서 맵을 생성하는 함수
void InitMap() {
    //이미 생성된 맵인지 확인(방문했던 맵인지 확인)
    if (visited[currentRoomX][currentRoomY]) {
        // 이미 저장된 데이터를 worldMap으로 복사해서 불러오기
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                worldMap[r][c] = worldData[currentRoomX][currentRoomY][r][c];
            }
        }
        return; //방문했던 맵이면 아래 코드를 실행하지 않고, 즉시 종료
    }

    //랜덤 맵 패턴 시드 생성
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
    int randomIdx = rand() % MAX_PATTERNS;

    //맵 초기화 및 패턴 생성, 전체 월드의 테두리(벽) 생성 (여기선 tile값만 조정, 실제 출력은 아래 DrawMap에서 구현)
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            //기본 바닥 설정
            int tile = 0;

            //월드 테두리 생성 (1: 투명 외곽벽)
            if (r == 0 && currentRoomY == 0) tile = 1;
            if (r == MAP_ROWS - 1 && currentRoomY == MAX_ROOMS_Y - 1) tile = 1;
            if (c == 0 && currentRoomX == 0) tile = 1;
            if (c == MAP_COLS - 1 && currentRoomX == MAX_ROOMS_X - 1) tile = 1;

            //맵 패턴 생성
            if (r > 0 && r < MAP_ROWS - 1 && c > 0 && c < MAP_COLS - 1) {
                tile = mapLayouts[randomIdx][r][c];
            }

            //결과물을 현재 맵(worldMap)과 월드 데이터(worldData)에 동시 저장
            worldMap[r][c] = tile;
            worldData[currentRoomX][currentRoomY][r][c] = tile;
        }
    }

    visited[currentRoomX][currentRoomY] = true;//방문 표시
}

//벽 충돌 체크 함수
int IsWall(float x, float y) {
    int col = (int)(x / TILE_SIZE);
    int row = (int)(y / TILE_SIZE);
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return 1;

    // 0=바닥(가능), 1=외곽벽(불가), 2=장애물(불가), 3=문(가능)
    int tile = worldMap[row][col];
    return tile == 1 || tile == 2;
}

//맵 그리기 함수
//위에서 만든 지도(worldMap)를 가지고 실제 그림 출력
void DrawMap(SDL_Renderer* renderer, SDL_Texture* mapBg, SDL_Texture* wallTex, SDL_Texture* borderTex) {
    // 배경 이미지 전체 화면에 렌더링
    if (mapBg) {
        SDL_RenderCopy(renderer, mapBg, NULL, NULL);
    }

    // 이미 렌더링한 타일 추적 (인접 타일을 하나의 이미지로 합치기 위함)
    bool processed[MAP_ROWS][MAP_COLS];
    memset(processed, 0, sizeof(processed));

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int tileType = worldMap[r][c];
            // 0=바닥, 1=외곽벽(투명), 3=문(통과 가능) → 렌더링 안 함
            // 2=맵 장애물만 텍스처 렌더링
            if (tileType != 2 || processed[r][c]) continue;

            // 오른쪽으로 최대 너비 확장 (같은 타일 타입만)
            int w = 1;
            while (c + w < MAP_COLS && worldMap[r][c + w] == tileType && !processed[r][c + w])
                w++;

            // 아래쪽으로 최대 높이 확장 (같은 너비·같은 타입의 벽 타일이 연속될 때만)
            int h = 1;
            while (r + h < MAP_ROWS) {
                int ok = 1;
                for (int k = 0; k < w; k++) {
                    if (worldMap[r + h][c + k] != tileType || processed[r + h][c + k]) {
                        ok = 0; break;
                    }
                }
                if (!ok) break;
                h++;
            }

            // 사용한 타일 전부 처리 완료 표시
            for (int dr = 0; dr < h; dr++)
                for (int dc = 0; dc < w; dc++)
                    processed[r + dr][c + dc] = true;

            SDL_Rect rect = { c * TILE_SIZE, r * TILE_SIZE, w * TILE_SIZE, h * TILE_SIZE };
            if (wallTex) {
                SDL_RenderCopy(renderer, wallTex, NULL, &rect);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

//시작 방 위치 (중앙)
int currentRoomX = 2;
int currentRoomY = 2;

//맵 끝까지 도달했을 경우, 다음 맵으로 이동하는 함수
void MoveToNextRoom(int direction) {

    //현재 맵의 몹 데이터 저장
    SaveEnemiesForRoom(currentRoomX, currentRoomY);

    //맵 좌표 변경
    if (direction == 0) currentRoomY--; //위
    else if (direction == 1) currentRoomY++; //아래
    else if (direction == 2) currentRoomX--; //왼쪽
    else if (direction == 3) currentRoomX++; //오른쪽

    LoadEnemiesForRoom(currentRoomX, currentRoomY); //다음 방의 몹 로드
    InitProjectiles(); //이전 방의 투사체 제거
    InitMap(); //다음 맵 생성(또는 불러오기)
}

// 해당 방향 가장자리에 있는 yellow 문 타일의 중앙 픽셀 좌표를 반환
// direction: 0=상(X 반환), 1=하(X 반환), 2=좌(Y 반환), 3=우(Y 반환)
// 문이 없으면 -1 반환
int GetDoorCenter(int direction) {
    int sum = 0, count = 0;

    if (direction == 0 || direction == 1) {
        for (int c = 0; c < MAP_COLS; c++) {
            for (int depth = 0; depth <= 6; depth++) {
                int r = (direction == 0) ? depth : (MAP_ROWS - 1 - depth);
                if (r < 0 || r >= MAP_ROWS) continue;
                if (worldMap[r][c] == 3) {
                    sum += c * TILE_SIZE + TILE_SIZE / 2;
                    count++;
                    break;
                }
            }
        }
    } else {
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int depth = 0; depth <= 6; depth++) {
                int c = (direction == 2) ? depth : (MAP_COLS - 1 - depth);
                if (c < 0 || c >= MAP_COLS) continue;
                if (worldMap[r][c] == 3) {
                    sum += r * TILE_SIZE + TILE_SIZE / 2;
                    count++;
                    break;
                }
            }
        }
    }

    return count > 0 ? sum / count : -1;
}

// 플레이어 히트박스(pos1~pos2) 범위가 해당 방향 가장자리의 문(type 3) 타일과 겹치는지 확인
// direction: 0=상, 1=하, 2=좌, 3=우  /  pos1~pos2: 벽과 평행한 축의 픽셀 범위
int HasDoorAtEdge(int direction, float pos1, float pos2) {
    int t1 = (int)(pos1 / TILE_SIZE);
    int t2 = (int)(pos2 / TILE_SIZE);
    if (t1 < 0) t1 = 0;
    if (t2 < 0) t2 = 0;

    for (int t = t1; t <= t2; t++) {
        for (int depth = 0; depth <= 6; depth++) {
            int r, c;
            if      (direction == 0) { r = depth;              c = t; }
            else if (direction == 1) { r = MAP_ROWS - 1 - depth; c = t; }
            else if (direction == 2) { r = t; c = depth;            }
            else                     { r = t; c = MAP_COLS - 1 - depth; }

            if (r < 0 || r >= MAP_ROWS || c < 0 || c >= MAP_COLS) continue;
            if (worldMap[r][c] == 3) return 1;
        }
    }
    return 0;
}


//미니맵 그리는 함수
void DrawMiniMap(SDL_Renderer* renderer) {
    //미니맵 설정 (오른쪽 상단 구석)
    int miniMapX = SCREEN_WIDTH - 120; //미니맵 시작 X 위치
    int miniMapY = 20;                 //미니맵 시작 Y 위치
    int roomSize = 15;                 //미니맵 속 방 한 칸의 크기
    int padding = 5;                   //방 사이 간격

    //미니맵 전체 배경 (해당 부분은 SDL2 전용 함수로, Gemini의 도움을 받음)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect bgRect = { miniMapX - 10, miniMapY - 10, 110, 110 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // 반투명 검정
    SDL_RenderFillRect(renderer, &bgRect);

    //맵들을 순회하며 그리기
    for (int y = 0; y < MAX_ROOMS_Y; y++) {
        for (int x = 0; x < MAX_ROOMS_X; x++) {
            SDL_Rect roomRect = {
                miniMapX + (x * (roomSize + padding)),
                miniMapY + (y * (roomSize + padding)),
                roomSize,
                roomSize
            };

            // 현재 내가 있는 방이면 노란색, 아니면 어두운 회색
            if (x == currentRoomX && y == currentRoomY) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 노란색
            }
            else {
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);  // 어두운 회색
            }

            SDL_RenderFillRect(renderer, &roomRect);
        }
    }
}
