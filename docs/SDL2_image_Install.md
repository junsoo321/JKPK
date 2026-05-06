# SDL2_image 설치 가이드 (Visual Studio 2022 x64)<br><br>

## 1. 라이브러리 다운로드
[SDL2_image 공식 GitHub](https://github.com/libsdl-org/SDL_image/releases)에 접속합니다.<br>
**SDL2_image-devel-2.8.12-VC.zip (Visual C++용)** 파일을 다운로드합니다.<br>
해당 폴더를 압축해제 후 나온 파일들을 기존의 SDL2 폴더에 덮어씌웁니다.
 - SDL2_image의 include안의 파일들을 SDL의 include 폴더로 이동 **($(ProjectDir)External\SDL2\include)**
 - SDL2_image의 lib/x64안의 파일들을 SDL의 lib/x64 폴더로 이동 **($(ProjectDir)External\SDL2\lib\x64)**

<br><br>
## 2. Visual Studio 프로젝트 설정<br>
### 1. 프로젝트 이름을 우클릭하고 **[속성(Properties)]**창을 엽니다.
<br>

### 2.라이브러리 파일 지정:
**링커 > 입력 > 추가 종속성(Additional Dependencies)**<br>
기존 입력 뒤에 SDL2_image.lib;을 추가 입력합니다.
전체 입력 : SDL2.lib; SDL2main.lib; SDL2_image.lib;
<br><br>


## 3. 실행 환경 구성 (중요)
압축 푼 폴더의 SDL2/lib/x64/SDL2_image.dll 파일을 복사하여 .exe 실행 파일이 생성되는 **($(ProjectDir)/x64/Debug)** 폴더에 붙여넣어야 합니다.
