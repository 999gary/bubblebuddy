@echo off
if exist himt.exe del himt.exe
if not exist build mkdir build
pushd build
::cl ..\source\bfbb_stat_tracker.c /DDOLPHIN /MT /Od /Oi /Zi /link /SUBSYSTEM:WINDOWS /incremental:no /opt:ref user32.lib gdiplus.lib shlwapi.lib d3d9.lib Comdlg32.lib
cl ..\source\main.c /Fehimt.exe /MT /Od /Oi /Zi /link /incremental:no /opt:ref user32.lib gdiplus.lib shlwapi.lib d3d9.lib Comdlg32.lib
if not errorlevel 0 goto EndBuild 
copy himt.exe ..
:EndBuild
popd

