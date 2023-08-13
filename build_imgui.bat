@echo off
REM make run as administrator work
pushd "%~dp0"

where /q cl
IF ERRORLEVEL 1 call "run_vcvarsall.bat"

set CommonCompilerFlags=-nologo -MTd -Zi -FC -GR- -EHa- -O2 -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505
set CommonLinkerFlags= -incremental:no -opt:ref -opt:icf

timeit.exe _start
IF NOT EXIST build mkdir build
pushd build
IF NOT EXIST imgui mkdir imgui
pushd imgui

cl -c %CommonCompilerFlags% ^
-I ..\..\code\imgui\ ^
-I ..\..\code\ ^
..\..\code\imgui\imgui.cpp ^
..\..\code\imgui\imgui_draw.cpp ^
..\..\code\imgui\imgui_widgets.cpp ^
..\..\code\imgui\imgui_tables.cpp

lib imgui.obj imgui_draw.obj imgui_widgets.obj imgui_tables.obj

popd
popd
echo|set /p=Compile took: 
timeit.exe _end
pause