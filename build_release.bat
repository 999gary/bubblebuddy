@echo off
if exist bubblebuddy.exe del bubblebuddy.exe
if not exist build mkdir build
pushd build
::cl ..\source\main.c /Febubblebuddy.exe /MT /O2 /Oi /link /incremental:no /opt:ref user32.lib gdiplus.lib shlwapi.lib d3d9.lib Comdlg32.lib
cl ..\source\main.c /DWIN32_NO_CRT /Febubblebuddy.exe /Gm- /GR- /EHa- /GS- /Gs9999999 /O2 /Oi /link /SUBSYSTEM:WINDOWS /STACK:0x100000,0x100000 /NODEFAULTLIB /incremental:no /opt:ref kernel32.lib shell32.lib user32.lib gdiplus.lib shlwapi.lib d3d9.lib Comdlg32.lib
if not errorlevel 0 goto EndBuild 
copy bubblebuddy.exe ..
:EndBuild
popd

