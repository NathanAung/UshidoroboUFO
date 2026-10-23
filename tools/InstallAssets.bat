@echo off

cd /d "%~dp0..\assets"

set modelsFolder="%~dp0..\Media\models"
set materialFolder="%~dp0..\Media\materials\scripts"
set textureFolder="%~dp0..\Media\materials\textures"
set particleFolder="%~dp0..\Media\particle"
set audioFolder="%~dp0..\Media\audio"

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