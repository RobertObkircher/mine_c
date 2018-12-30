cmake_minimum_required(VERSION 3.13)
project(mine_c C)

set(CMAKE_C_STANDARD 11)

add_executable(mine_c src/main.c src/mathc-master/mathc.c)
target_link_libraries(mine_c glfw OpenGL m)
