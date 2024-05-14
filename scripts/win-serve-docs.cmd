@echo off
setlocal enabledelayedexpansion
pushd %~dp0\..\docs
call ..\tools\mdbook\windows\mdbook serve --open
endlocal
popd
exit 0