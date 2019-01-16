#include <stdio.h>
#include <glad/glad.h>
#include <mathc.h>
#include <GLFW/glfw3.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>

#include "camera.h"
#include "chunks.h"
#include "filereload.h"
#include "shader.h"
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    Camera cam = default_camera;
    cam.position[0] = center;
    cam.position[1] = center;
    cam.position[2] = center;

    int fly = 1;

    while (!glfwWindowShouldClose(window)) {
        double t = glfwGetTime();
        double dt = t - last;
        last = t;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = (float) width / height;

        cam.aspect = ratio;
        update_camera(&cam);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader1);

        float projection_view[MAT4_SIZE];
        mat4_multiply(projection_view, cam.perspective, cam.view);
        render_chunks(projection_view);

        glfwSwapBuffers(window);

        update_filereload();

        glfwPollEvents();

        float player_speed = (float) dt;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            move_camera(&cam, cam.forward, 3 * player_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            move_camera(&cam, cam.forward, -2 * player_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            rotate_camera(&cam, 0, player_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            rotate_camera(&cam, 0, -player_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            rotate_camera(&cam, player_speed, 0);
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            rotate_camera(&cam, -player_speed, 0);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            move_camera(&cam, (float[]) {0, 1, 0}, 2 * player_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            move_camera(&cam, (float[]) {0, 1, 0}, -2 * player_speed);
        }
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            glfwSetCursorPos(window, 0, 0);

            rotate_camera(&cam, -(float) ypos * player_speed, -(float) xpos * player_speed);
        }

        if (glfwGetKey(window, GLFW_KEY_F)) {
            fly = !fly;
        }
        if (!fly) {
            int height = -10;
            for (unsigned int y = cam.position[1]; y >= 0; --y) {
                if (block_at(cam.position[0], y, cam.position[2])) {
                    height = y;
                    break;
                }
            }
            cam.position[1] = fmaxf(height + 2, cam.position[1] - player_speed);
        }

    }

    delete_shader_programs();
    close_filereload();
    glfwTerminate();
    return 0;
}

