#include <stdio.h>
#include <glad/glad.h>
#include <mathc.h>
#include <GLFW/glfw3.h>

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

static const char *vertex_shader_text =
        "uniform mat4 MVP;\n"
        "attribute vec3 vCol;\n"
        "attribute vec3 vPos;\n"
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = MVP * vec4(vPos, 1.0);\n"
        "    color = vCol;\n"
        "}\n";

static const char *fragment_shader_text =
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(color, 1.0);\n"
        "}\n";

int main(void) {
    glfwSetErrorCallback(gl_error_callback);

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

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint mvp_location = glGetUniformLocation(program, "MVP");
    GLint vpos_location = glGetAttribLocation(program, "vPos");
    GLint vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray((GLuint) vpos_location);
    glVertexAttribPointer((GLuint) vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 6, (void *) 0);

    glEnableVertexAttribArray((GLuint) vcol_location);
    glVertexAttribPointer((GLuint) vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 6, (void *) (sizeof(float) * 3));

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

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
