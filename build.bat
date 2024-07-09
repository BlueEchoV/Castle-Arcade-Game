@echo off
echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64

echo Checking and creating the object file directory...
if not exist "C:\Projects\Castle-Arcade-Game\obj\" mkdir "C:\Projects\Castle-Arcade-Game\obj\"

echo Compiling each source file...
for %%f in (C:\Projects\Castle-Arcade-Game\src\*.cpp) do (
    cl /std:c++20 /EHsc /I"C:\Projects\Castle-Arcade-Game\contrib\sdl\include" /I"C:\Projects\Castle-Arcade-Game\contrib\stb" ^
       /Fo"C:\Projects\Castle-Arcade-Game\obj\%%~nf.obj" /c %%f
    if errorlevel 1 (
        echo Compilation of %%f failed.
        pause
        exit /b 1
    )
)

echo Linking...
link /OUT:"C:\Projects\Castle-Arcade-Game\output.exe" /LIBPATH:"C:\Projects\Castle-Arcade-Game\contrib\sdl\lib\x64" ^
     C:\Projects\Castle-Arcade-Game\obj\*.obj SDL2.lib SDL2main.lib

if errorlevel 1 (
    echo Linking failed.
    pause
    exit /b 1
)

echo Build successful.
pause