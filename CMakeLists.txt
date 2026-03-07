cmake_minimum_required(VERSION 3.14)
project(CMeshStudio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt6 path
set(Qt6_DIR "E:/Qt/6.10.1/mingw_64/lib/cmake/Qt6")

# Set build directory
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Include directories
include_directories(
    include
    extlib/nlohmann-json-develop/include
    extlib/libtcc/include
)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/mainwindow.cpp
    src/math_types.cpp
    src/geometry.cpp
    src/tccengine.cpp
    src/glviewport.cpp
    src/csyntaxhighlighter.cpp
)

# Header files
set(HEADER_FILES
    include/mainwindow.h
    include/math_types.h
    include/geometry.h
    include/tccengine.h
    include/glviewport.h
    include/csyntaxhighlighter.h
)

# Add executable
add_executable(CMeshStudio ${SOURCE_FILES} ${HEADER_FILES})

# Link libraries
target_link_libraries(CMeshStudio
    Qt6::Widgets
    Qt6::OpenGLWidgets
    ${CMAKE_SOURCE_DIR}/extlib/libtcc/lib/libtcc.a
)

# Set compiler flags
if(MSVC)
    target_compile_options(CMeshStudio PRIVATE /W4 /WX)
    set_target_properties(CMeshStudio PROPERTIES WIN32_EXECUTABLE TRUE)
else()
    target_compile_options(CMeshStudio PRIVATE -Wall -Wextra -Wpedantic)
    target_link_options(CMeshStudio PRIVATE -Wl,-subsystem,windows)
endif()
