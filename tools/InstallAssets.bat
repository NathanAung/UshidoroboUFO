@echo off

cd /d "%~dp0..\assets"

set mediaPath="%~dp0..\Media"

if not exist %mediaPath%\audio md %mediaPath%\audio

set modelsFolder="%mediaPath%\models"
set materialFolder="%mediaPath%\materials\scripts"
set textureFolder="%mediaPath%\materials\textures"
set particleFolder="%mediaPath%\particle"
set audioFolder="%mediaPath%\audio"

echo Importing assets into OGRE media...

for %%f in (*) do (
    echo File: %%f -- Extension: %%~xf
    
    if /i "%%~xf"==".mesh" (
		copy %%f %modelsFolder%
    )
    if /i "%%~xf"==".material" (
        copy %%f %materialFolder%
    )
    if /i "%%~xf"==".png" (
        copy %%f %textureFolder%
    )
    if /i "%%~xf"==".jpg" (
        copy %%f %textureFolder%
    )
	if /i "%%~xf"==".particle" (
        copy %%f %particleFolder%
    )
	if /i "%%~xf"==".mp3" (
        copy %%f %audioFolder%
    )
	if /i "%%~xf"==".wav" (
        copy %%f %audioFolder%
    )
	if /i "%%~xf"==".ogg" (
        copy %%f %audioFolder%
    )
	if /i "%%~xf"==".flac" (
        copy %%f %audioFolder%
    )
)

echo.
echo Process finished!
pause

exit