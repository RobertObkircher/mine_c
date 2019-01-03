#include <stdio.h>
#include <glad/glad.h>
#include <mathc.h>
#include <GLFW/glfw3.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>

#include "filereload.h"
#include "shader.h"

static void gl_error_callback(const int error, const char *description) {
    fprintf(stderr, "gl_error_callback: %d, %s\n", error, description);
}

typedef struct {
    float x, y, z, r, g, b;
} Vertex;

static const Vertex vertices[] = {
        {-0.3f, 0.8f,  0.0f, 1.0f, 0.0f, 0.0f},
        {-0.7f, -0.3f, 0.0f, 0.0f, 1.0f, 0.0f},
        {0.9f,  -0.6f, 0.0f, 0.0f, 0.0f, 1.0f},
};

static GLuint shader1 = 0;
static GLuint mvp_location;

static void shader1_callback(char *path) {
    shader1 = compile_shaders_and_link_program(shader1, path);

    mvp_location = glGetUniformLocation(shader1, "MVP");

    // TODO this doesn't work when reloading
    GLint vpos_location = glGetAttribLocation(shader1, "vPos");
    GLint vcol_location = glGetAttribLocation(shader1, "vCol");

    glEnableVertexAttribArray((GLuint) vpos_location);
    glVertexAttribPointer((GLuint) vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 6, (void *) 0);

    glEnableVertexAttribArray((GLuint) vcol_location);
    glVertexAttribPointer((GLuint) vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 6, (void *) (sizeof(float) * 3));
}


int main(int argc, char *argv[]) {
    init_filereload();

    glfwSetErrorCallback(gl_error_callback);

    log_trace("asdf");
    log_debug("asdf");
    log_warn("asdf");

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

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    listen_for_file_changes("assets", "shader1.glsl", shader1_callback);
    shader1_callback("assets/shader1.glsl"); // TODO option to invoke callback immediately

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = (float) width / height;

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);

        float m[MAT4_SIZE];
        mat4_identity(m);
        mat4_rotation_z(m, (mfloat_t) (glfwGetTime()));

        float p[MAT4_SIZE];
        mat4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);

        float mvp[MAT4_SIZE];
        mat4_multiply(mvp, p, m);

        glUseProgram(shader1);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);

        update_filereload();

        glfwPollEvents();
    }

    delete_shader_programs();
    close_filereload();
    glfwTerminate();
    return 0;
}

