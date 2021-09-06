@echo off
if exist gfl.exe del gfl.exe
if not exist build mkdir build
pushd build
::cl ..\source\bfbb_stat_tracker.c /DDOLPHIN /MT /Od /Oi /Zi /link /SUBSYSTEM:WINDOWS /incremental:no /opt:ref user32.lib gdiplus.lib shlwapi.lib d3d9.lib Comdlg32.lib
cl ..\source\scenedata\generate_formatted_list.c /Fegfl.exe /MT /Od /Oi /Zi /link /incremental:no /opt:ref user32.lib gdiplus.lib shlwapi.lib d3d9.lib Comdlg32.lib
if not errorlevel 0 goto EndBuild 
copy gfl.exe ..
:EndBuild
popd

