@echo off

:: Set paths
set CMAKE_PATH=E:\Qt\Tools\CMake_64\bin
set MINGW_PATH=E:\Qt\Tools\mingw1310_64\bin
set NINJA_PATH=E:\Qt\Tools\Ninja
set PATH=%PATH%;%CMAKE_PATH%;%MINGW_PATH%;%NINJA_PATH%

:: Create build directory
if not exist build mkdir build

:: Build project
echo Building project...
cd build

:: Run CMake with Ninja generator
%CMAKE_PATH%\cmake.exe .. -G "Ninja" -DCMAKE_CXX_COMPILER=%MINGW_PATH%\g++.exe -DCMAKE_C_COMPILER=%MINGW_PATH%\gcc.exe -DCMAKE_MAKE_PROGRAM=%NINJA_PATH%\ninja.exe -DCMAKE_BUILD_TYPE=Release

:: Build with Ninja
%NINJA_PATH%\ninja.exe

:: Check build result
if exist CMeshStudio.exe (
    echo Build successful!
) else (
    echo Build failed!
)

cd ..
pause
