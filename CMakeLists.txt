cmake_minimum_required(VERSION 3.13)
project(mine_c C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(dependencies)

add_executable(mine_c
        src/main.c
        src/filereload.c src/filereload.h
        src/shader.c src/shader.h
        src/allocorexit.c src/allocorexit.h
        src/chunks.c src/chunks.h
        src/textures.c src/textures.h
        src/files.c src/files.h
        src/camera.c src/camera.h
        src/list.h)

target_link_libraries(mine_c
        glfw
        m
        glad
        mathc
        dl
        logc
        png
        simplex)

target_compile_options(mine_c PRIVATE -Wall -Wextra -pedantic )
