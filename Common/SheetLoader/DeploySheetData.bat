@echo off
echo ================================
echo Google Sheets Data Deploy Script
echo ================================

:: 현재 디렉토리를 SheetLoader로 변경
cd /d "%~dp0"

echo 1. Google Sheets에서 데이터 다운로드 중...
dotnet run
if errorlevel 1 (
    echo 오류: Google Sheets 데이터 다운로드 실패
    pause
    exit /b 1
)

echo.
echo 2. 대상 폴더 생성 중...

:: GameServer/Resources/Data 폴더 생성
if not exist "..\..\GameServer\Resources\Data" (
    mkdir "..\..\GameServer\Resources\Data"
    echo - GameServer/Resources/Data 폴더 생성됨
)

:: TestClientUnity/Assets/Resources/Data 폴더 생성
if not exist "..\..\TestClientUnity\Assets\Resources" (
    mkdir "..\..\TestClientUnity\Assets\Resources"
)
if not exist "..\..\TestClientUnity\Assets\Resources\Data" (
    mkdir "..\..\TestClientUnity\Assets\Resources\Data"
    echo - TestClientUnity/Assets/Resources/Data 폴더 생성됨
)

echo.
echo 3. JSON 파일 복사 중...

:: resources 폴더의 모든 JSON 파일을 대상 위치에 복사
for %%f in (resources\*.json) do (
    echo - %%f 복사 중...
    copy "%%f" "..\..\GameServer\Resources\Data\" > nul
    copy "%%f" "..\..\TestClientUnity\Assets\Resources\Data\" > nul
    if errorlevel 1 (
        echo 오류: %%f 복사 실패
    ) else (
        echo   → GameServer/Resources/Data/
        echo   → TestClientUnity/Assets/Resources/Data/
    )
)

echo.
echo 4. 배포 완료!
echo - GameServer: %cd%\..\..\GameServer\Resources\Data\
echo - TestClientUnity: %cd%\..\..\TestClientUnity\Assets\Resources\Data\

echo.
echo 배포된 파일 목록:
dir "..\..\GameServer\Resources\Data\*.json" /b 2>nul
if errorlevel 1 (
    echo (JSON 파일 없음)
)

echo.
pause