#ifndef MINE_C_SHADER_H
#define MINE_C_SHADER_H

#endif //MINE_C_SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint compile_shaders_and_link_program(GLuint id, char *filepath);

void delete_shader_programs();
