cmake_minimum_required(VERSION 3.14)
project(video-app C CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

# add_definitions(-DGL_SILENCE_DEPRECATION)

include_directories(src/)

list(APPEND SOURCES
    src/bin/main.cpp
    src/app/app.cpp
    src/base/command_line.cpp
    src/egl/aegl.cpp
    src/egl/utils.cpp
    src/gles2/egl/dma_buffer_texture.cpp
    src/gles2/egl/eglext.cpp
    src/gles2/shader.cpp
    src/gles2/texture.cpp
    src/gles2/utils.cpp
    src/v4l2/v4l2_device.cpp
    src/window/awindow_x11.cpp
)

list(APPEND BINARY_SOURCES
    src/app/shader/a.vert
    src/app/shader/a.frag
)

# MACOSX_BUNDLE WIN32 ? These are out of target
add_executable(app-main ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/src/app/shader/a.vert.o
    ${CMAKE_CURRENT_BINARY_DIR}/src/app/shader/a.frag.o
)

target_compile_options(app-main PUBLIC -O2 -Wall)
target_compile_options(app-main PUBLIC -g)

# list(APPEND EXTRA_LIBS
#     "-lGLESv2 -lEGL -lX11"
# )

target_link_libraries(app-main Dependencies ${EXTRA_LIBS})


foreach(binary_source IN LISTS BINARY_SOURCES)
    # FIXME: an workaround for removing absolte path in symbol
    # 理想は _binary_src_app_shader_a_vert_start のようなシンボル名
    # redefine-sym するしか無い？
    # python スクリプトで cmake 外に出すとか
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${binary_source}.o
        COMMAND cp ARGS ${CMAKE_SOURCE_DIR}/${binary_source} ${binary_source}
        COMMAND ${CMAKE_OBJCOPY} ARGS
        -I binary -O elf64-x86-64 -B i386:x86-64
        ${binary_source} ${binary_source}.o
        DEPENDS ${CMAKE_SOURCE_DIR}/${binary_source}
    )

endforeach(binary_source)
