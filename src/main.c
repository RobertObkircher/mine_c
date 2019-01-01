#include <stdio.h>
#include <glad/glad.h>
#include <mathc.h>
#include <GLFW/glfw3.h>

static void error_callback(const int error, const char* description) {
   fprintf(stderr, "error_callback: %d, %s\n", error, description);
}

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(0);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);


    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        //

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
