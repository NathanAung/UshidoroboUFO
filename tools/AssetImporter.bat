@echo off

set converter="%~dp0..\OGRE\bin\OgreAssimpConverter.exe"



if "%~1"=="" (
    echo NO FILE SELECTED
    echo Please drag and drop a 3D model file onto this script.
    echo.
    pause
    exit /b
)

cd /d "%~dp1"

echo Converting %~nx1 to OGRE asset...
"%converter%" %~nx1

echo.
echo Conversion finished!
pause

exit