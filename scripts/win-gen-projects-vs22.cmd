@echo off
pushd %~dp0\..\
call tools\premake\windows\premake5.exe vs2022
popd
pause