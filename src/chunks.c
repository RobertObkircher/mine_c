#include "chunks.h"
#include "dynarray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <mathc.h>
#include <log.h>
#include <open-simplex-noise.h>

typedef struct {
    Block data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
} Blocks;
static const Blocks empty_blocks;

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int z;
    GLuint vao;
    GLuint vertex_buffer;
    GLuint index_buffer;
    unsigned short number_of_indices; // This probably should be an int
    unsigned int needs_mesh_update : 1;
} ChunkInfo;

// Visible chunks
static int visible_chunks_count;
static int visible_chunks_size;
static Blocks *visible_chunks_blocks;
static ChunkInfo *visible_chunks_infos;

// TODO invisible chunks
//static int invisible_chunks_count;
//static Blocks *invisible_chunks_blocks;

typedef struct {
    unsigned int is_valid : 1;
    unsigned int is_visible : 1;
    unsigned int is_compressed : 1;
    unsigned int pos : 30;
} ChunkPointer;

//TODO index offset
static ChunkPointer chunk_index[HORIZONTAL_CHUNKS][VERTICAL_CHUNKS][HORIZONTAL_CHUNKS];

struct osn_context *osn;
void setup_world_generator() {
    open_simplex_noise(123, &osn);
}
void destroy_world_generator() {
    open_simplex_noise_free(osn);
}

void make_visible_chunk(int x, int y, int z) {
    realloc_if_too_small((void **) &visible_chunks_blocks, sizeof(Blocks), visible_chunks_size,
                         visible_chunks_count + 1);
    visible_chunks_size = realloc_if_too_small((void **) &visible_chunks_infos, sizeof(ChunkInfo), visible_chunks_size,
                                               visible_chunks_count + 1);

    Blocks blocks = empty_blocks;
    for (int ix = 0; ix < CHUNK_SIZE; ++ix) {
        for (int iy = 0; iy < CHUNK_SIZE; ++iy) {
            for (int iz = 0; iz < CHUNK_SIZE; ++iz) {
                int block_x = x + ix;
                int block_y = y + iy;
                int block_z = z + iz;
                float level = (float)block_y / (CHUNK_SIZE * VERTICAL_CHUNKS) - 0.5f;

                float density = (float)open_simplex_noise3(osn, block_x,block_y,block_z) - 4 * level;

                if (density > 0.5) {
                    blocks.data[ix][iy][iz] = 2;
                } else if (density > 0.3) {
                    blocks.data[ix][iy][iz] = 1;
                }
//                if (y + iy < CHUNK_SIZE * VERTICAL_CHUNKS / 2) {
//                    blocks.data[ix][iy][iz] = 2;
//                } else if (y + iy == CHUNK_SIZE * VERTICAL_CHUNKS / 2) {
//                    blocks.data[ix][iy][iz] = rand() & 1;
//                }
            }
        }
    }

    chunk_index[x / CHUNK_SIZE][y / CHUNK_SIZE][z /
                                                CHUNK_SIZE] = (ChunkPointer) {.is_valid = 1, .is_visible = 1, .pos = visible_chunks_count};

    visible_chunks_blocks[visible_chunks_count] = blocks;
    visible_chunks_infos[visible_chunks_count++] = (ChunkInfo) {.x = x, .y = y, .z = z, .needs_mesh_update = 1};
}


static int is_culled(ChunkInfo *info) {
    return 0; // TODO
}

//
// Mesh stuff
//

typedef struct {
    float x, y, z, u, v;
//    float nx, ny, nz; // TODO normals
} ChunkVertex;

typedef unsigned short ChunkIndex;

static ChunkIndex vertex_buffer_size;
static ChunkIndex vertex_buffer_elements;
static ChunkVertex *vertex_buffer;
static size_t index_buffer_size;
static int index_buffer_elements;
static ChunkIndex *index_buffer;

/*
 * 0----3
 * |    |
 * |    |
 * 1----2
 *
 * Indices: 012 023
 */
static void add_quad(Block block, ChunkVertex v0, ChunkVertex v1, ChunkVertex v2, ChunkVertex v3) {
    //
    // Apply texture coordinates TODO move somewhere else
    //
    int num_tiles = 2;
    float tile_size = 1.0f / num_tiles;

    v0.u = v1.u = 0;
    v2.u = v3.u = tile_size;

    v0.v = v3.v = tile_size;
    v1.v = v2.v = 0;

    int x_offset;
    int y_offset;
    switch (block) {
        case 1: {
            x_offset = 0;
            y_offset = 0;
            break;
        }
        case 2: {
            x_offset = 1;
            y_offset = 0;
            break;
        }
        default: {
            x_offset = 1;
            y_offset = 1;
        }
    }
    v0.u += x_offset * tile_size;
    v1.u += x_offset * tile_size;
    v2.u += x_offset * tile_size;
    v3.u += x_offset * tile_size;

    v0.v += y_offset * tile_size;
    v1.v += y_offset * tile_size;
    v2.v += y_offset * tile_size;
    v3.v += y_offset * tile_size;

    //
    // Add quad
    //

    // TODO move to update_mesh and increase growth
    index_buffer_size = realloc_if_too_small((void **) &index_buffer, sizeof(ChunkIndex), index_buffer_size,
                                             index_buffer_elements + 6);
    vertex_buffer_size = (ChunkIndex) realloc_if_too_small((void **) &vertex_buffer, sizeof(ChunkVertex),
                                                           vertex_buffer_size,
                                                           vertex_buffer_elements + 4);

    index_buffer[index_buffer_elements++] = vertex_buffer_elements;
    index_buffer[index_buffer_elements++] = vertex_buffer_elements + (ChunkIndex) 1;
    index_buffer[index_buffer_elements++] = vertex_buffer_elements + (ChunkIndex) 2;
    index_buffer[index_buffer_elements++] = vertex_buffer_elements;
    index_buffer[index_buffer_elements++] = vertex_buffer_elements + (ChunkIndex) 2;
    index_buffer[index_buffer_elements++] = vertex_buffer_elements + (ChunkIndex) 3;

    vertex_buffer[vertex_buffer_elements++] = v0;
    vertex_buffer[vertex_buffer_elements++] = v1;
    vertex_buffer[vertex_buffer_elements++] = v2;
    vertex_buffer[vertex_buffer_elements++] = v3;
}


extern GLuint shader1; // TODO remove
extern GLuint the_texture; // TODO remove

void add_block(int x, int y, int z, Block current, Block next_x, Block next_y, Block next_z) {
    ChunkVertex v0, v1, v2, v3;

    if (current) {
        // This block is opaque, so the other blocks are invisible
        if (!next_x) {
            v0.x = v1.x = v2.x = v3.x = x + 0.5f;

            v0.y = v3.y = y + 0.5f;
            v1.y = v2.y = y - 0.5f;

            v0.z = v1.z = z + 0.5f;
            v2.z = v3.z = z - 0.5f;

            add_quad(current, v0, v1, v2, v3);
        }
        if (!next_y) {
            v0.x = v1.x = x - 0.5f;
            v2.x = v3.x = x + 0.5f;

            v0.y = v1.y = v2.y = v3.y = y + 0.5f;

            v0.z = v3.z = z - 0.5f;
            v1.z = v2.z = z + 0.5f;

            add_quad(current, v0, v1, v2, v3);
        }
        if (!next_z) {
            // 0--3
            // |  |
            // 1--2
            // This face is easy to think about
            // What changes when rotation around x or y?
            v0.x = v1.x = x - 0.5f;
            v2.x = v3.x = x + 0.5f;

            v0.y = v3.y = y + 0.5f;
            v1.y = v2.y = y - 0.5f;

            v0.z = v1.z = v2.z = v3.z = z + 0.5f;

            add_quad(current, v0, v1, v2, v3);
        }
    } else {
        // This is an air block, so the other blocks are visible
        // These are the same faces as above, but the triangles need to be drawn clockwise
        // This can easily be achieved by swapping v1 and v3.
        // The inner ifs need to be inverted too
        if (next_x) {
            v0.x = v1.x = v2.x = v3.x = x + 0.5f;

            v0.y = v3.y = y + 0.5f;
            v1.y = v2.y = y - 0.5f;

            v0.z = v1.z = z + 0.5f;
            v2.z = v3.z = z - 0.5f;

            add_quad(next_x, v0, v3, v2, v1);
        }
        if (next_y) {
            v0.x = v1.x = x - 0.5f;
            v2.x = v3.x = x + 0.5f;

            v0.y = v1.y = v2.y = v3.y = y + 0.5f;

            v0.z = v3.z = z - 0.5f;
            v1.z = v2.z = z + 0.5f;

            add_quad(next_y, v0, v3, v2, v1);
        }
        if (next_z) {
            v0.x = v1.x = x - 0.5f;
            v2.x = v3.x = x + 0.5f;

            v0.y = v3.y = y + 0.5f;
            v1.y = v2.y = y - 0.5f;

            v0.z = v1.z = v2.z = v3.z = z + 0.5f;

            add_quad(next_z, v0, v3, v2, v1);
        }
    }
}

Block block_at(unsigned int x, unsigned int y, unsigned int z) {
    unsigned int chunk_x = x / CHUNK_SIZE;
    unsigned int chunk_y = y / CHUNK_SIZE;
    unsigned int chunk_z = z / CHUNK_SIZE;

    if (chunk_x >= 0 && chunk_x < HORIZONTAL_CHUNKS
        && chunk_y >= 0 && chunk_y < VERTICAL_CHUNKS
        && chunk_z >= 0 && chunk_z < HORIZONTAL_CHUNKS) {

        ChunkPointer *p = &chunk_index[chunk_x][chunk_y][chunk_z];
        if (p && p->is_valid && p->is_visible) {
            Blocks *blocks = &visible_chunks_blocks[p->pos];
            return blocks->data[x - chunk_x * CHUNK_SIZE][y - chunk_y * CHUNK_SIZE][z - chunk_z * CHUNK_SIZE];
        }
    }
    return 0;
}

static void update_mesh(int index, ChunkInfo *info) {
    // TODO limit update time per frame

    Blocks *blocks = &visible_chunks_blocks[index];

    vertex_buffer_elements = 0;
    index_buffer_elements = 0;

    //
    // Add blocks within the chunk
    //

    for (int x = 0; x < CHUNK_SIZE - 1; ++x) {
        for (int y = 0; y < CHUNK_SIZE - 1; ++y) {
            for (int z = 0; z < CHUNK_SIZE - 1; ++z) {
                Block current = blocks->data[x][y][z];
                Block next_x = blocks->data[x + 1][y][z];
                Block next_y = blocks->data[x][y + 1][z];
                Block next_z = blocks->data[x][y][z + 1];

                add_block(x, y, z, current, next_x, next_y, next_z);
            }
        }
    }

    //
    // Border to next chunk in direction X
    //
    Blocks *next_x_blocks = NULL;
    if (info->x / CHUNK_SIZE < HORIZONTAL_CHUNKS - 1) {
        ChunkPointer *next_x_chunk = &chunk_index[info->x / CHUNK_SIZE + 1][info->y / CHUNK_SIZE][info->z / CHUNK_SIZE];
        if (next_x_chunk->is_valid && next_x_chunk->is_visible)
            next_x_blocks = &visible_chunks_blocks[next_x_chunk->pos];
    }

    if (next_x_blocks) {
        int x = CHUNK_SIZE - 1;
        for (int y = 0; y < CHUNK_SIZE - 1; ++y) {
            for (int z = 0; z < CHUNK_SIZE - 1; ++z) {
                Block current = blocks->data[x][y][z];
                Block next_x = next_x_blocks->data[0][y][z];
                Block next_y = blocks->data[x][y + 1][z];
                Block next_z = blocks->data[x][y][z + 1];

                add_block(x, y, z, current, next_x, next_y, next_z);
            }
        }
    }

    //
    // Border to next chunk in direction Y
    //
    Blocks *next_y_blocks = NULL;
    if (info->y / CHUNK_SIZE < VERTICAL_CHUNKS - 1) {
        ChunkPointer *next_y_chunk = &chunk_index[info->x / CHUNK_SIZE][info->y / CHUNK_SIZE + 1][info->z / CHUNK_SIZE];
        if (next_y_chunk->is_valid && next_y_chunk->is_visible)
            next_y_blocks = &visible_chunks_blocks[next_y_chunk->pos];
    }

    if (next_y_blocks) {
        int y = CHUNK_SIZE - 1;
        for (int x = 0; x < CHUNK_SIZE - 1; ++x) {
            for (int z = 0; z < CHUNK_SIZE - 1; ++z) {
                Block current = blocks->data[x][y][z];
                Block next_x = blocks->data[x + 1][y][z];
                Block next_y = next_y_blocks->data[x][0][z];
                Block next_z = blocks->data[x][y][z + 1];

                add_block(x, y, z, current, next_x, next_y, next_z);
            }
        }
    }

    //
    // Border to next chunk in direction Z
    //

    Blocks *next_z_blocks = NULL;
    if (info->z / CHUNK_SIZE < HORIZONTAL_CHUNKS - 1) {
        ChunkPointer *next_z_chunk = &chunk_index[info->x / CHUNK_SIZE][info->y / CHUNK_SIZE][info->z / CHUNK_SIZE + 1];
        if (next_z_chunk->is_valid && next_z_chunk->is_visible)
            next_z_blocks = &visible_chunks_blocks[next_z_chunk->pos];
    }

    if (next_z_blocks) {
        int z = CHUNK_SIZE - 1;
        for (int x = 0; x < CHUNK_SIZE - 1; ++x) {
            for (int y = 0; y < CHUNK_SIZE - 1; ++y) {
                Block current = blocks->data[x][y][z];
                Block next_x = blocks->data[x + 1][y][z];
                Block next_y = blocks->data[x][y + 1][z];
                Block next_z = next_z_blocks->data[x][y][0];

                add_block(x, y, z, current, next_x, next_y, next_z);
            }
        }
    }

    //
    // Corners
    //

    if (next_x_blocks && next_y_blocks) {
        int a = CHUNK_SIZE - 1;
        for (int z = 0; z < CHUNK_SIZE - 1; ++z) {
            Block current = blocks->data[a][a][z];
            Block next_x = next_x_blocks->data[0][a][z];
            Block next_y = next_y_blocks->data[a][0][z];
            Block next_z = blocks->data[a][a][z + 1];

            add_block(a, a, z, current, next_x, next_y, next_z);
        }
    }
    if (next_x_blocks && next_z_blocks) {
        int a = CHUNK_SIZE - 1;
        for (int y = 0; y < CHUNK_SIZE - 1; ++y) {
            Block current = blocks->data[a][y][a];
            Block next_x = next_x_blocks->data[0][y][a];
            Block next_y = blocks->data[a][y + 1][a];
            Block next_z = next_z_blocks->data[a][y][0];

            add_block(a, y, a, current, next_x, next_y, next_z);
        }
    }
    if (next_y_blocks && next_z_blocks) {
        int a = CHUNK_SIZE - 1;
        for (int x = 0; x < CHUNK_SIZE - 1; ++x) {
            Block current = blocks->data[x][a][a];
            Block next_x = blocks->data[x + 1][a][a];
            Block next_y = next_y_blocks->data[x][0][a];
            Block next_z = next_z_blocks->data[x][a][0];

            add_block(x, a, a, current, next_x, next_y, next_z);
        }
    }

    if (next_x_blocks && next_y_blocks && next_z_blocks) {
        int a = CHUNK_SIZE - 1;
        Block current = blocks->data[a][a][a];
        Block next_x = next_x_blocks->data[0][a][a];
        Block next_y = next_y_blocks->data[a][0][a];
        Block next_z = next_z_blocks->data[a][a][0];

        add_block(a, a, a, current, next_x, next_y, next_z);
    }

    //
    // OpenGL
    //

    if (!info->vao) {
        glGenVertexArrays(1, &info->vao);
    }
    glBindVertexArray(info->vao);

    if (!info->vertex_buffer) {
        glGenBuffers(1, &info->vertex_buffer);
    }
    glBindBuffer(GL_ARRAY_BUFFER, info->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_elements * sizeof(ChunkVertex), vertex_buffer, GL_STATIC_DRAW);

    GLint vpos_location = glGetAttribLocation(shader1, "vPos");
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 0);
    glEnableVertexAttribArray(vpos_location);

    GLint uv_location = glGetAttribLocation(shader1, "vUV");
    glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(uv_location);

    if (!info->index_buffer) {
        glGenBuffers(1, &info->index_buffer);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_elements * sizeof(ChunkIndex), index_buffer, GL_STATIC_DRAW);

    info->number_of_indices = index_buffer_elements;
    info->needs_mesh_update = 0;
}

void render_chunks(float projection_view[]) {
    // TODO setup shader
    glUseProgram(shader1);

    float mvp[MAT4_SIZE];
    float model[MAT4_SIZE];
    float pos[VEC3_SIZE];

    GLuint mvp_location = glGetUniformLocation(shader1, "MVP");

    for (int i = 0; i < visible_chunks_count; ++i) {
        ChunkInfo *info = &visible_chunks_infos[i];

        if (is_culled(info))
            continue;

        if (info->needs_mesh_update)
            update_mesh(i, info);

        mat4_identity(model);
        mat4_translation(model, model, vec3(pos, info->x, info->y, info->z));
        mat4_multiply(mvp, projection_view, model);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *) mvp);

        glBindVertexArray(info->vao);
        glDrawElements(GL_TRIANGLES, info->number_of_indices, GL_UNSIGNED_SHORT, 0);
    }
}


