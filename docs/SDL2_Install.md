# SDL2 환경 설정 가이드 (Visual Studio 2022 x64)

## 1. 라이브러리 다운로드
[SDL2 공식 GitHub](https://github.com/libsdl-org/SDL/releases)에 접속합니다.
**Development Libraries** 섹션에서 **SDL2-devel-2.x.x-VC.zip (Visual C++용)** 파일을 다운로드합니다.
압축을 풀면 include 폴더와 lib 폴더가 나옵니다. 이 폴더들을 프로젝트 폴더 내 External/SDL2 위치에 압축 해제합니다.

2. Visual Studio 프로젝트 설정
상단 설정이 All Configurations / x64인지 반드시 확인하세요!

헤더 경로 설정:

프로젝트 속성 > C/C++ > 일반 > 추가 포함 디렉터리

$(ProjectDir)External\SDL2\include 추가

라이브러리 경로 설정:

프로젝트 속성 > 링커 > 일반 > 추가 라이브러리 디렉터리

$(ProjectDir)External\SDL2\lib\x64 추가

라이브러리 파일 지정:

프로젝트 속성 > 링커 > 입력 > 추가 종속성

SDL2.lib; SDL2main.lib; 입력 (맨 앞에 추가)

시스템 설정:

프로젝트 속성 > 링커 > 시스템 > 하위 시스템

콘솔 (/SUBSYSTEM:CONSOLE) 설정 (printf 디버깅용)

3. 실행 환경 구성 (중요)
External/SDL2/lib/x64/SDL2.dll 파일을 복사하여 .vcxproj 파일이 있는 폴더(또는 실행 파일이 생성되는 x64/Debug 폴더)에 붙여넣어야 합니다.
