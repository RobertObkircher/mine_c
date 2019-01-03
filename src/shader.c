#include "shader.h"
#include <log.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SHADER_PROGRAMS 100
static GLuint programs[MAX_SHADER_PROGRAMS];
static int program_count = 0;

void delete_shader_programs() {
    while (program_count > 0)
        glDeleteProgram(programs[--program_count]);
}

static char *read_file_contents(char *path) {
    FILE *file = fopen(path, "r");
    if (!file || fseek(file, 0, SEEK_END))
        return NULL;

    size_t length = (size_t) ftell(file);
    rewind(file);

    char *buffer = malloc(length + 1);
    if (buffer) {
        size_t read = fread(buffer, 1, length, file);
        buffer[read] = 0;
    }

    fclose(file);

    return buffer;
}

GLuint compile_shaders_and_link_program(GLuint id, char *filepath) {
    log_info("Loading: %s", filepath);

    char *shader_text = read_file_contents(filepath);
    if (!shader_text) {
        log_error("unable to read file: %s", filepath);
        exit(1);
    }
    size_t shader_text_len = strlen(shader_text);

    // TODO error checks:

    char *vertex_shader_source[] = {"#define COMPILING_VERTEX_SHADER\n", shader_text};
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 2, vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    char *fragment_shader_source[] = {"#define COMPILING_FRAGMENT_SHADER\n", shader_text};
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 2, fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    GLuint program = id;
    if (!program) {
        if (program_count >= MAX_SHADER_PROGRAMS) {
            log_error("too many shaders");
            exit(1);
        }
        program = glCreateProgram(); // TODO error check
        programs[program_count++] = program;
    }

    // TODO error checks:
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    glDeleteShader(vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(fragment_shader);

    free(shader_text);
    return program;
}
