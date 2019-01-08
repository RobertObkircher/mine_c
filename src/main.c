#include <stdio.h>
#include <glad/glad.h>
#include <mathc.h>
#include <GLFW/glfw3.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>

#include "filereload.h"
#include "shader.h"
#include "chunks.h"

static void gl_error_callback(const int error, const char *description) {
    fprintf(stderr, "gl_error_callback: %d, %s\n", error, description);
}

GLuint shader1 = 0;

static void shader1_callback(char *path) {
    shader1 = compile_shaders_and_link_program(shader1, path);
}

// TODO opengl debugging: https://stackoverflow.com/a/43567924

int main(void) {
    init_filereload();

    glfwSetErrorCallback(gl_error_callback);

    int test_world_size = 5;

    for (int x = 0; x < test_world_size; ++x) {
        for (int y = 0; y < test_world_size / 2; ++y) {
            for (int z = 0; z < test_world_size; ++z) {
                make_visible_chunk(CHUNK_SIZE * x, CHUNK_SIZE * y, CHUNK_SIZE * z);
            }
        }
    }

    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(0);

    listen_for_file_changes("assets", "shader1.glsl", shader1_callback);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = (float) width / height;

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);

        float projection_view[MAT4_SIZE];
        {
            float perspective[MAT4_SIZE];
            mat4_perspective(perspective, 3.1415f / 3.0f, ratio, 0.1, 100);

            float position[VEC3_SIZE];
            float target[VEC3_SIZE];
            float up[VEC3_SIZE];
            float view[MAT4_SIZE];

            float center = test_world_size * CHUNK_SIZE / 2.0f;
            mat4_look_at(view,
                         vec3(position, center, center, center),
                         vec3(target, center, center / 2, center / 2),
                         vec3(up, 0.0, 1.0, 0.0));

            mat4_multiply(projection_view, perspective, view);
        }

        glUseProgram(shader1);

        render_chunks(projection_view);

        glfwSwapBuffers(window);

        update_filereload();

        glfwPollEvents();
    }

    delete_shader_programs();
    close_filereload();
    glfwTerminate();
    return 0;
}

