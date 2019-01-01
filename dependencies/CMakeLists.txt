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


# log.c
add_library(
        logc # If the library name would contain a dot mathc stops compiling when add_compile_definition is used
        log.c/src/log.c
)
add_compile_definitions(logc PUBLIC LOG_USE_COLOR)
target_include_directories(logc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/log.c/src")
