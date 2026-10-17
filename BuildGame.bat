@echo off

if exist "%localappdata%/w64cmake/bin" set PATH=%localappdata%/w64cmake/bin;%PATH%

cmake -S %~dp0 -B %~dp0bin -G "Visual Studio 17 2022" -A x64

cmake --build %~dp0bin --config Release


if %errorlevel% neq 0 (
		color 0c
		echo Build failed!
pause
) else (
		color 0a
		echo Build succeeded. Launching game...
		cd bin/Release
		MyApp.exe
)