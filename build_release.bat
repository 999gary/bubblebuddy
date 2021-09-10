@echo off
if exist bubblebuddy.exe del bubblebuddy.exe
if not exist build mkdir build
pushd build
cl ..\source\main.c /Febubblebuddy.exe /MT /O2 /Oi /link /incremental:no /opt:ref user32.lib gdiplus.lib shlwapi.lib d3d9.lib Comdlg32.lib
if not errorlevel 0 goto EndBuild 
copy bubblebuddy.exe ..
:EndBuild
popd

