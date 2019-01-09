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
#include "textures.h"

static void gl_error_callback(const int error, const char *description) {
    fprintf(stderr, "gl_error_callback: %d, %s\n", error, description);
}

GLuint shader1 = 0;
GLuint the_texture = 0;

static void shader1_callback(char *path) {
    GLuint new_shader = compile_shaders_and_link_program(shader1, path);
    if (new_shader)
        shader1 = new_shader;
}

static void load_the_texture(char *path) {
    GLuint new_texture = load_png_texture_from_path(path);
    if (new_texture) {
        GLuint old_texture = the_texture;
        the_texture = new_texture;
        if (old_texture)
            glDeleteTextures(1, &old_texture);

        log_info("Texture reload successful: %s", path);
    } else {
        log_error("Texture reload failed: %s", path);
    }
}

// TODO opengl debugging: https://stackoverflow.com/a/43567924

static struct {
    float position[VEC3_SIZE];
    float direction[VEC3_SIZE];
} player;

int main(void) {
    init_filereload();

    glfwSetErrorCallback(gl_error_callback);

    for (int x = 0; x < HORIZONTAL_CHUNKS; ++x) {
        for (int y = 0; y < VERTICAL_CHUNKS; ++y) {
            for (int z = 0; z < HORIZONTAL_CHUNKS; ++z) {
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    listen_for_file_changes("assets", "shader1.glsl", shader1_callback);
    listen_for_file_changes("assets", "image.png", load_the_texture);

    double last = glfwGetTime();

    float center = HORIZONTAL_CHUNKS * CHUNK_SIZE / 2.0f;
    vec3(player.direction, 0, 0, -1);
    vec3(player.position, center, center, center);

    while (!glfwWindowShouldClose(window)) {
        double t = glfwGetTime();
        double dt = t - last;
        last = t;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = (float) width / height;

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float projection_view[MAT4_SIZE];
        {
            float perspective[MAT4_SIZE];
            mat4_perspective(perspective, 3.1415f / 3.0f, ratio, 0.1, 100);

            float up[VEC3_SIZE];
            float view[MAT4_SIZE];

            float target[VEC3_SIZE];
            vec3_assign(target, player.position);
            vec3_add(target, target, player.direction);

            mat4_look_at(view, player.position, target, vec3(up, 0.0, 1.0, 0.0));

            mat4_multiply(projection_view, perspective, view);
        }

        glUseProgram(shader1);

        render_chunks(projection_view);

        glfwSwapBuffers(window);

        update_filereload();

        glfwPollEvents();

        float player_speed = (float) dt;
        if (glfwGetKey(window, GLFW_KEY_W)) {
            float delta[VEC3_SIZE];
            vec3_multiply_f(delta, player.direction, player_speed * 2);
            vec3_add(player.position, player.position, delta);
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            float delta[VEC3_SIZE];
            vec3_multiply_f(delta, player.direction, -player_speed);
            vec3_add(player.position, player.position, delta);
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            float rotation[MAT3_SIZE];
            mat3_identity(rotation);
            mat3_rotation_y(rotation, player_speed);
            vec3_multiply_mat3(player.direction, player.direction, rotation);
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            float rotation[MAT3_SIZE];
            mat3_identity(rotation);
            mat3_rotation_y(rotation, -player_speed);
            vec3_multiply_mat3(player.direction, player.direction, rotation);
        }
    }

    delete_shader_programs();
    close_filereload();
    glfwTerminate();
    return 0;
}

