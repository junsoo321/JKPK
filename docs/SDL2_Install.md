# SDL2 설치 가이드 (Visual Studio 2022 x64)<br><br>

## 1. 라이브러리 다운로드
[SDL2 공식 GitHub](https://github.com/libsdl-org/SDL/releases)에 접속합니다.<br>
**SDL2-devel-2.32.10-VC.zip (Visual C++용)** 파일을 다운로드합니다.<br>
압축을 풀면 include 폴더와 lib 폴더가 나옵니다. (해당 폴더를 프로젝트 폴더로 이동시킵니다. **경로에 한글이 없는지 확인**)
<br><br>
## 2. Visual Studio 프로젝트 설정<br>
### 1. 프로젝트 이름을 우클릭하고 **[속성(Properties)]**창을 엽니다.
<br>

### 2. 헤더 파일 연결 (Include)
**C/C++ > 일반 > 추가 포함 디렉터리(Additional Include Directories)**<br>
압축을 푼 SDL2 폴더 내부의 include 폴더 경로 **($(ProjectDir)External\SDL2\include)** 추가
<br><br>

### 3.라이브러리 경로 설정:
**링커 > 일반 > 추가 라이브러리 디렉터리(Additional Library Directories)**<br>
SDL2 폴더 내부의 lib\x64 폴더 경로 **($(ProjectDir)External\SDL2\lib\x64)** 추가
<br><br>

### 4.라이브러리 파일 지정:
**링커 > 입력 > 추가 종속성(Additional Dependencies)**<br>
SDL2.lib; SDL2main.lib; 입력 (맨 앞에 추가)
<br><br>

### 5.시스템 설정:
**링커 > 시스템 > 하위 시스템**<br>
**창(/SUBSYSTEM:WINDOWS)** 설정
<br><br>

## 3. 실행 환경 구성 (중요)
압축 푼 폴더의 SDL2/lib/x64/SDL2.dll 파일을 복사하여 .exe 실행 파일이 생성되는 **($(ProjectDir)/x64/Debug)** 폴더에 붙여넣어야 합니다.
<br><br>
## 4. 설치 확인 테스트 코드
[테스트 코드](SDL2_test.c)를 main.cpp에 넣고 빌드했을때 검은 창이 떴다가 2초 후 꺼진다면 설치 성공입니다.
