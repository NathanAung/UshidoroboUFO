@echo off

cd /d "%~dp0..\assets"

set modelsFolder="%~dp0..\Media\models"
set materialFolder="%~dp0..\Media\materials\scripts"
set textureFolder="%~dp0..\Media\materials\textures"
set particleFolder="%~dp0..\Media\particle"

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
)

echo.
echo Process finished!
pause

exit