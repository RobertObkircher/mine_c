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

GLuint shader2 = 0;

static void shader2_callback(char *path) {
    GLuint new_shader = compile_shaders_and_link_program(shader2, path);
    if (new_shader)
        shader2 = new_shader;
}

// TODO opengl debugging: https://stackoverflow.com/a/43567924

int main(void) {
    init_filereload();
    glfwSetErrorCallback(gl_error_callback);

    setup_world_generator();
    for (unsigned int x = 0; x < HORIZONTAL_CHUNKS; ++x) {
        for (unsigned int y = 0; y < VERTICAL_CHUNKS; ++y) {
            for (unsigned int z = 0; z < HORIZONTAL_CHUNKS; ++z) {
                make_visible_chunk((ChunkPos) {.x = x, .y = y, .z = z});
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
    listen_for_file_changes("assets", "shader2.glsl", shader2_callback);
    listen_for_file_changes("assets", "image.png", load_the_texture);

    double last = glfwGetTime();

    float center = HORIZONTAL_CHUNKS * CHUNK_SIZE / 2.0f;

    Camera cam = default_camera;
    cam.position[0] = center;
    cam.position[1] = VERTICAL_CHUNKS * CHUNK_SIZE / 1.8f;
    cam.position[2] = center;

    int fly = 1;

    glUseProgram(shader2);
    GLint shader2_mvp_location = glGetUniformLocation(shader2, "MVP");

    GLuint shader2_vao;
    glGenVertexArrays(1, &shader2_vao);
    glBindVertexArray(shader2_vao);
    glEnableVertexAttribArray(0);

    GLuint shader2_vertex_pos_buffer;
    glGenBuffers(1, &shader2_vertex_pos_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shader2_vertex_pos_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLfloat vertices[] = {
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };
    for (int i = 0; i < 36 * 3; ++i) {
        vertices[i] /= 4;
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

        {
            float pos[VEC3_SIZE];
            int collides = 0;
            vec3_assign(pos, cam.position);
            for (float i = 0; i < 100; i += 0.1) {
                BlockPos p = (BlockPos) {
                        .x = (unsigned int) roundf(pos[0]),
                        .y = (unsigned int) roundf(pos[1]),
                        .z = (unsigned int) roundf(pos[2]),
                };
                if (block_at(p)) {
                    collides = 1;
                    break;
                }
                float delta[VEC3_SIZE];
                vec3_multiply_f(delta, cam.forward, i);
                vec3_add(pos, pos, delta);
            }
            if (collides) {
                glUseProgram(shader2);
                glBindVertexArray(shader2_vao);
                float model[MAT4_SIZE];
                float mvp[MAT4_SIZE];
                mat4_identity(model);
                mat4_translation(model, model, pos);
                mat4_multiply(mvp, projection_view, model);
                glUniformMatrix4fv(shader2_mvp_location, 1, GL_FALSE, mvp);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }


        glfwSwapBuffers(window);

        update_filereload();

        glfwPollEvents();

        float player_speed = (float) dt;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            move_camera(&cam, cam.forward, 5 * player_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            move_camera(&cam, cam.forward, -5 * player_speed);
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
            for (unsigned int y = (unsigned int) cam.position[1]; y >= 0; --y) {
                BlockPos p = (BlockPos) {
                        .x = (unsigned int) cam.position[0],
                        .y = y,
                        .z = (unsigned int) cam.position[2]
                };
                if (block_at(p)) {
                    height = y;
                    break;
                }
            }
            cam.position[1] = fmaxf(height + 2, cam.position[1] - player_speed);
        }
        center_world_at((unsigned int) cam.position[0], (unsigned int) cam.position[2], CHUNK_SIZE);

    }

    destroy_world_generator();
    delete_shader_programs();
    close_filereload();
    glfwTerminate();
    return 0;
}

