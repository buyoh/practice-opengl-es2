cmake_minimum_required(VERSION 3.14)
project(video-app C CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

# add_definitions(-DGL_SILENCE_DEPRECATION)

include_directories(src/)

list(APPEND SOURCES
    src/bin/main.cpp
    src/app/app.cpp
    src/egl/aegl.cpp
    src/gles2/shader.cpp
    src/gles2/texture.cpp
    src/gles2/utils.cpp
    src/window/awindow_x11.cpp
)

# MACOSX_BUNDLE WIN32 ? These are out of target
add_executable(app-main ${SOURCES})

target_compile_options(app-main PUBLIC -O2 -Wall)
target_compile_options(app-main PUBLIC -g)

# list(APPEND EXTRA_LIBS
#     "-lGLESv2 -lEGL -lX11"
# )

target_link_libraries(app-main Dependencies ${EXTRA_LIBS})
