# Set paths
$CMAKE_PATH = "E:\Qt\Tools\CMake_64\bin"
$MINGW_PATH = "E:\Qt\Tools\mingw1310_64\bin"
$NINJA_PATH = "E:\Qt\Tools\Ninja"
$env:PATH += ";$CMAKE_PATH;$MINGW_PATH;$NINJA_PATH"

# Create build directory
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Build project
Write-Host "Building project..."
Set-Location "build"

# Run CMake with Ninja generator
& "$CMAKE_PATH\cmake.exe" .. -G "Ninja" -DCMAKE_CXX_COMPILER="$MINGW_PATH\g++.exe" -DCMAKE_C_COMPILER="$MINGW_PATH\gcc.exe" -DCMAKE_MAKE_PROGRAM="$NINJA_PATH\ninja.exe" -DCMAKE_BUILD_TYPE=Release

# Build with Ninja
& "$NINJA_PATH\ninja.exe"

# Check build result
if (Test-Path "CMeshStudio.exe") {
    Write-Host "Build successful!"
} else {
    Write-Host "Build failed!"
}

Set-Location ".."
Read-Host "Press Enter to exit..."
