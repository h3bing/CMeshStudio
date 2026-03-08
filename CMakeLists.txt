cmake_minimum_required(VERSION 3.16)

project(CMeshStudio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets OpenGL OpenGLWidgets REQUIRED)

include_directories(include)
include_directories(extlib/libtcc/include)
include_directories(extlib/nlohmann-json-develop/include)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/math_types.cpp
    src/geometry.cpp
    src/tccengine.cpp
    src/glviewport.cpp
    src/csyntaxhighlighter.cpp
)

set(HEADERS
    include/mainwindow.h
    include/math_types.h
    include/geometry.h
    include/tccengine.h
    include/glviewport.h
    include/csyntaxhighlighter.h
)

add_executable(CMeshStudio ${SOURCES} ${HEADERS})

target_link_libraries(CMeshStudio PRIVATE Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
target_link_libraries(CMeshStudio PRIVATE ${CMAKE_SOURCE_DIR}/extlib/libtcc/lib/libtcc.a)


