# glad
add_library(
        glad
        glad/src/glad.c
)
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")


# mathc
add_library(
        mathc
        mathc/mathc.c
)
target_include_directories(mathc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/mathc")