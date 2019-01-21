#include "shader.h"
#include "allocorexit.h"
#include "dynarray.h"
#include "files.h"
#include <log.h>
#include <stdlib.h>
#include <string.h>

static GLuint *programs;
static size_t programs_count;
static size_t programs_size;

void delete_shader_programs() {
    while (programs_count > 0)
        glDeleteProgram(programs[--programs_count]);
}

static GLuint create_and_compile_shader(const char *sources[], int count, GLenum type) {
    GLuint shader = glCreateShader(type);
    if (!shader || shader == GL_INVALID_ENUM) {
        log_error("unable to create shader");
        return 0;
    }
    glShaderSource(shader, count, sources, NULL);
    glCompileShader(shader);
    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE) {
        int length = 1024;
        char info_log[length];
        glGetShaderInfoLog(shader, length, &length, info_log);
        log_error("vertex shader info log: %s", info_log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}


GLuint compile_shaders_and_link_program(GLuint id, char *filepath) {
    log_info("Loading shader: %s", filepath);

    char *shader_text = read_file_contents(filepath);
    if (!shader_text)
        return 0;

    const char version[] = "#version 460\n";

    const char *vertex_shader_sources[] = {version, "#define COMPILING_VERTEX_SHADER\n", shader_text};
    GLuint vertex_shader = create_and_compile_shader(vertex_shader_sources, 3, GL_VERTEX_SHADER);
    if (!vertex_shader)
        return 0;

    const char *fragment_shader_sources[] = {version, "#define COMPILING_FRAGMENT_SHADER\n", shader_text};
    GLuint fragment_shader = create_and_compile_shader(fragment_shader_sources, 3, GL_FRAGMENT_SHADER);
    if (!fragment_shader) {
        glDeleteShader(vertex_shader);
        return 0;
    }

    GLuint program = id;
    if (!program) {
        program = glCreateProgram();
        if (!program) {
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            log_error("unable to create program");
            return 0;
        }
        programs_size = realloc_if_too_small((void **) &programs, sizeof(GLuint), programs_size, ++programs_count);
        programs[programs_count - 1] = program;
    }

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    glDeleteShader(vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(fragment_shader);

    int program_link_status;
    glGetProgramiv(program, GL_LINK_STATUS, &program_link_status);
    if (program_link_status != GL_TRUE) {
        int length = 1024;
        char info_log[length];
        glad_glGetProgramInfoLog(program, length, &length, info_log);
        if (!id)
            glDeleteProgram(program);
        log_error("program info log: %s", info_log);
        return 0;
    }

    free(shader_text);
    return program;
}
