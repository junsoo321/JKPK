# 개발 로그

---

## 2026-06-14 ~ 2026-06-15

### 비주얼 / UI

#### 타이틀 화면 배경
- `assets/ui/menu/main.png`를 타이틀 화면 전체 배경으로 적용
- 이미지 로드 실패 시 기존 텍스트 폴백 유지

#### 플레이어 스프라이트 방향 수정
- `walk.png` / `idle.png` 스프라이트 시트 행(row) 순서 재매핑
  - row 0 : 앞(아래) / row 1 : 옆(오른쪽) / row 2 : 뒤(위)
- 좌우 이동 시 `SDL_FLIP_HORIZONTAL`로 반전 처리

#### 체력바 표시 조건 수정
- `GAME_MAZE` 및 `GAME_MAZE_TRANSITION` 상태에서는 체력바 숨김
- 미로 스테이지 진입 중에는 얼굴 아이콘(거리 기반 5단계)이 대신 표시됨

---

### 미로 스테이지

#### 화살표 에셋 적용
- `assets/map/maze/ui/arrow.png`를 출구 방향 화살표 블록에 적용
- `SDL_RenderCopyEx`로 방향에 따라 회전 렌더링

#### 얼굴 아이콘 5단계 거리 표시
- 플레이어와 출구 사이 거리를 전체 대각선 기준으로 5구간으로 나눔
- `character_icon_1.png`(울음, 멀 때) ~ `character_icon_5.png`(웃음, 가까울 때)
- 에셋 경로: `assets/map/maze/ui/character_icon_1~5.png`
- 미로 스테이지 진입 시 좌상단에 96×96 크기로 표시

#### 미로 진입 트랜지션 효과
- 미로 방 진입 시 화면 스핀+축소 효과 적용 (1.5초)
- `SDL_RenderReadPixels`로 현재 화면을 캡처 후 회전·축소하며 빨려들어가는 연출
- 흰색 플리커 효과 병행

---

### 충돌 맵 통합

- 기존 보스맵·일반맵에 각각 따로 존재하던 collision 로딩 로직을 `LoadCollisionFromPNG` 단일 함수로 통합
- 색상 규칙: 마젠타(`#FF00FF`) = 벽(1), 노란색(`#FFFF00`) = 장애물(2)

---

### 디버그 메뉴 (Debug 빌드 전용)

#### 비밀번호 입력
- `Shift + T`로 비밀번호 입력창 활성화 (게임 플레이 중에만 작동)
- 비밀번호: `1234`
- 잘못 입력 시 1초간 오류 메시지 표시

#### 메뉴 UI
- 비밀번호 인증 후 방향키 내비게이션 메뉴 표시
  - `UP/DOWN` : 항목 선택
  - `RIGHT` : 확인 / 서브메뉴 진입
  - `LEFT` : 뒤로가기
- 키 안내는 메뉴 박스 바깥 하단에 표시
- 이후 `Shift + T` 재입력 시 메뉴 즉시 재오픈

#### 맵 텔레포트
- 메뉴: `Map → Boss / Maze / Quiz / Plain`
- 선택 즉시 해당 타입 맵 생성 및 진입
  - **Boss** : 보스 초기화 및 격리벽 생성 후 `GAME_BOSS` 진입
  - **Maze** : `StartMazeStage()` 직접 호출, 트랜지션 없이 `GAME_MAZE` 즉시 진입
  - **Quiz** : `StartQuizStage()` 호출 후 `GAME_QUIZ` 진입
  - **Plain** : `InitMap()` + `LoadEnemiesForRoom()` 후 `GAME_NORMAL` 진입
- 메뉴가 열려 있는 동안 게임 업데이트(플레이어·적·스테이지) 일시 정지
- `GAME_MAZE` 상태에서도 디버그 메뉴 사용 가능

#### 버그 수정
- 디버그로 미로 진입 시 이전 방의 몬스터가 남아있던 문제 수정
  - `roomType = ROOM_MAZE` 설정 후 `LoadEnemiesForRoom` 호출로 적 배열 초기화

---

### 전투 시스템

#### 보스 투사체 에셋 분리
- `FireBossProjectile(startX, startY, targetX, targetY, texIndex)` 함수 신설
- 기존 `FireEnemyProjectile`이 보스 패턴에서도 호출되어 일반 몬스터 이미지를 쓰던 문제 수정
- texIndex 체계:

| texIndex | 텍스처 | 사용 주체 |
|----------|--------|----------|
| 0 | `mob/projectile/boss-001.png` | 보스 |
| 1 | `mob/projectile/boss-002.png` | 보스 (3페이즈) |
| 2 | `mob/projectile/default.png` | 일반 몬스터 |
| 3 | `mob/projectile/ninja.png` | 닌자 몬스터 |

#### 몬스터 타입별 공격 패턴 차별화
- **일반(Normal)** : 단일 조준탄, 랜덤 인터벌 (기존 동작 유지)
- **닌자(Ninja)** : 3초 주기 대시 + **2.5초 주기 ±15° 3방향 확산탄** 추가
  - `FireEnemyProjectileEx` 함수 신설 (`texIndex` 파라미터 지정 가능)
  - 확산탄은 닌자 전용 이미지(`ninja.png`) 적용
- **자폭(Suicide)** : 근접 폭발 유지, 투사체 없음

#### 몬스터 타입별 스프라이트 분리
- 기존 단일 `gEnemyTexture` + colorMod 방식에서 타입별 독립 텍스처로 변경

| 타입 | 에셋 경로 |
|------|----------|
| Normal | `assets/mob/enemy/normal.png` |
| Ninja | `assets/mob/enemy/ninja.png` |
| Suicide | `assets/mob/enemy/suicide.png` |

- 각 파일 미존재 시 `mob/enemy.png`로 폴백
- 자폭 몬스터 근접 경고 붉은 점멸 효과는 새 텍스처에도 유지

---

### 추가 에셋 경로 정리

새로 추가된 에셋은 아래 경로에 배치:

```
assets/
├── ui/menu/main.png                        # 타이틀 배경
├── map/maze/ui/arrow.png                   # 미로 방향 화살표
├── map/maze/ui/character_icon_1~5.png      # 거리 표시 아이콘
├── mob/enemy/
│   ├── normal.png
│   ├── ninja.png
│   └── suicide.png
└── mob/projectile/
    ├── default.png                         # 일반 몬스터 투사체
    ├── ninja.png                           # 닌자 투사체
    ├── boss-001.png                        # 보스 투사체 1
    └── boss-002.png                        # 보스 투사체 2
```

---

## 2026-06-15 (이어서)

### 에셋 폴더 구조 전면 재편 (`c0cdb49`)

전체 에셋 경로를 의미 단위로 통일. 주요 경로 변경:

| 이전 경로 | 변경 후 경로 |
|-----------|-------------|
| `assets/maze/*` | `assets/map/maze/*` |
| `assets/display/heart_*.png` | `assets/ui/hp/heart_*.png` |
| `assets/Characters/Blue/*` | `assets/player/body/blue/*` |
| `assets/attack.png` | `assets/mob/projectile/default.png` |
| `assets/ui/main.png` | `assets/ui/menu/main.png` |
| `assets/Pretendard-Regular.ttf` | `assets/ui/font/Pretendard-Regular.ttf` |

- `ImageManager.cpp`의 중복 이미지 로드 함수 통합
- `GameState.hpp`에 `GAME_SETTINGS` 상태 추가

---

### 퀴즈 스테이지 에셋 및 몬스터 분리 (`0ec43ea`, `9fa4d51`)

#### 몬스터 이미지 타입별 분리
- `enemy.png` → `normal.png`, `mob-ninja.png` → `ninja.png`, `mob-bomb.png` → `suicide.png`

#### 퀴즈 스테이지 에셋 추가
- `assets/map/quiz/computer_off.png`, `computer_on.png`
- `assets/map/quiz/quiz_1.png`, `quiz_2.png`, `quiz_3.png`

#### 디버그 메뉴 몬스터 스폰 기능 추가 (`9fa4d51`)
- 디버그 메뉴 "Spawn Monster" 서브메뉴에서 특정 타입 몬스터 즉시 소환
- `BossPattern.cpp`: 보스 투사체 함수(`FireBossProjectile`)로 교체 완료

---

### 충돌 맵 다양화 + 장애물 에셋 (`15b7bbc`)

- `map-base-001_collision-A.png`, `-B.png`, `-C.png` 3종 추가 (룸 생성 시 랜덤 선택)
- 열린 문 에셋 4방향: `assets/map/door/door-open-up/down/left/right.png`
- 장애물 이미지: `assets/map/obstacle.png`
- `ImageManager.cpp` / `MapSystem.cpp`: 충돌 맵 배리에이션 처리 및 장애물 렌더링 적용

---

### 자폭 몬스터 메커니즘 + 투사체 이미지 정리 (`8b454a9`)

- `semi-colon.png` (플레이어 투사체) 이미지 경계 제거 (128KB → 117KB)
- `Enemy.cpp`: 자폭 몬스터 근접 폭발 범위 및 판정 개선

---

### 폰트 크래시 수정 + 퀴즈 이미지 최적화 (`4821a04`, `4ad06fa`)

- 퀴즈 컴퓨터 이미지 압축 최적화 (약 5MB → 2MB)
- 닌자 전용 투사체 이미지 추가: `assets/mob/projectile/ninja.png`
- 폰트 로드 실패 시 크래시 수정 (`main.cpp` 폴백 처리)
- 디버그 메뉴에서 퀴즈 스테이지 진입 시 발생하던 마이너 버그 수정 (`4ad06fa`)

---

### 타이틀 화면 로고 이미지 (`883d31d`)

- `assets/ui/menu/title_logo.png` 추가
- `TitleScreen.cpp`: 타이틀 화면 상단에 로고 이미지 렌더링
- `ImageManager.hpp/cpp`에 로고 텍스처(`gTitleLogoTex`) 추가

---

### 아이템 시스템 (`caa81d5`)

#### 새 파일
- `include/Item.hpp` / `src/Item.cpp`

#### 구조

```
enum ItemType {
    ITEM_MONSTER_ULTRA_ENERGY = 0,  // 이동속도 +30%
    ITEM_COUNT
};
```

- `ApplyItem(player, type)` : 효과 적용 + 인벤토리 기록
- `GetItemAssetName(type)` : 에셋 파일명 반환 (→ `assets/player/item/` 기준)
- `GetItemName` / `GetItemDescription` : UI 표시용 문자열

#### 아이템 추가 방법 (주석으로 명시됨)
1. `Item.hpp`의 `ItemType` 열거형에 항목 추가
2. `Item.cpp` `ApplyItem` / `GetItemAssetName` 케이스 추가
3. `assets/player/item/` 에 PNG 에셋 추가
4. `ImageManager.cpp`가 자동 로드

#### 연결된 변경
- `Player.hpp/cpp`: `collectedItems[MAX_INVENTORY]`, `itemCount` 필드 추가
- `PauseMenu.cpp`: 일시정지 화면에 아이템 보드(`assets/ui/pause/item-board.png`) 및 아이콘 목록 렌더링
  - 보드 위치: `(150, 45)`, 크기: `500×200`
  - 아이콘 슬롯: `40×40`, 행당 8개, 간격 10px
- `RoomNode.h`: 룸별 아이템 필드 추가
- `MapSystem.cpp`: 룸 배치 시 아이템 연동

#### 새 에셋
- `assets/player/item/monster-ultra-energy.png`
- `assets/map/table.png` (룸 내 아이템 테이블 오브젝트)
- `assets/ui/pause/item-board.png` (인벤토리 배경 보드)
- `assets/ui/menu/title_logo.png` (타이틀 로고, 이전 커밋에서 추가)

---

### 설정 화면 — 폰트 선택 (`26092fb`)

#### 새 파일
- `include/SettingsScreen.hpp` / `src/SettingsScreen.cpp`

#### 동작
- `assets/ui/font/` 디렉토리의 `.ttf` 파일 목록을 런타임에 스캔 (`WIN32_FIND_DATAA`)
- 리스트 형태로 표시 (한 번에 7개 표시, 마우스 휠 / 방향키 스크롤)
- 항목 클릭 → `ReloadFont(fontName)` 호출 → 즉시 폰트 변경
- 현재 선택 항목은 초록 하이라이트, 마우스오버는 흰색

#### 진입 경로
- 타이틀 화면 → "설정" 버튼 → `GAME_SETTINGS` 상태 전환
- 뒤로 버튼 클릭 → `GAME_TITLE` 복귀

#### 연결된 변경
- `TitleScreen.cpp`: Settings 버튼 이벤트 처리 추가
- `main.cpp`: `GAME_SETTINGS` 분기 추가 (업데이트/렌더 루프)

---

### 현재 GameState 전체 목록

```cpp
GAME_TITLE
GAME_SETTINGS       // ← 오늘 추가
GAME_STORY
GAME_HELP
GAME_NORMAL
GAME_MAZE_TRANSITION
GAME_MAZE
GAME_QUIZ_PROMPT
GAME_QUIZ
GAME_BOSS
GAME_PAUSE
GAME_OVER_FADE
GAME_OVER
```

---

### 집에서 이어받을 때 확인 사항

- [ ] `assets/ui/font/` 에 `.ttf` 파일이 있어야 폰트 설정 화면이 동작함 (없으면 "폰트 파일을 찾을 수 없습니다." 표시)
- [ ] 아이템 효과(`ApplyItem`)가 실제 게임 플레이 중 올바르게 누적되는지 테스트 필요
- [ ] 충돌 맵 배리에이션(-A/-B/-C) 이 룸 생성 시 실제로 랜덤 선택되는지 확인
- [ ] 퀴즈 스테이지 진입 시 디버그 메뉴 관련 마이너 버그 수정 후 재검증 필요
