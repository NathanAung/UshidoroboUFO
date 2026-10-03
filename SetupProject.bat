@echo off

set ogreVersion=14.5.2
set vstVersion=142
set buildAll=1


:FileCheck
if exist "%cd%/Assets" set buildAll=0
if not exist "%localappdata%/w64cmake" goto InstallCMake
if exist "%cd%/OGRE.zip" goto ExitProgram


:SetupProject
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0setup/InstallOgre.ps1" -Command %buildAll% "%ogreVersion%" "%vstVersion%"
echo Project Setup completed...
pause
exit

:ExitProgram
echo Project is already installed...
pause
exit

:InstallCMake
echo Installing Gcc...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0setup/InstallCMake.ps1"
echo Waiting for CMake to finish installing...
pause
goto FileCheck


