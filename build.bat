@echo off
REM make run as administrator work
pushd "%~dp0"

where /q cl
IF ERRORLEVEL 1 call "\run_vcvarsall.bat"

REM 4244 is a useful warning
set CommonCompilerFlags=-nologo -utf-8 -diagnostics:caret -Zi -Zc:inline -FC -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4820 -wd5045 -wd4061 -wd4244 ..\code\win32_goldsrctosource.cpp
set CommonLinkerFlags= -OUT:goldsrctosource.exe -subsystem:console -incremental:no -opt:ref -opt:icf kernel32.lib shell32.lib

timeit.exe _start
IF NOT EXIST build mkdir build


set debug=1
if %debug% == 1 (

sokol-shdc.exe --input code/shaders/world.glsl --output code/shaders_compiled/world.h %ShdcParams% --slang glsl330:hlsl4:metal_macos -b
sokol-shdc.exe --input code/shaders/wire.glsl --output code/shaders_compiled/wire.h %ShdcParams% --slang glsl330:hlsl4:metal_macos -b

pushd build
cl -Od -GS -sdl -guard:cf -RTCu -DGC_DEBUG -DDEBUG_GRAPHICS -RTC1 -fsanitize=address -MTd %CommonCompilerFlags% /link %CommonLinkerFlags% -guard:cf imgui/imgui.lib libvcruntimed.lib libucrtd.lib

) else (

pushd build
cl -O2 -GS- %CommonCompilerFlags% /link %CommonLinkerFlags% libvcruntime.lib

)


popd
echo|set /p=Compile took: 
timeit.exe _end
