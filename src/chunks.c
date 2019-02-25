#include "chunks.h"
#include "list.h"

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
    ChunkPos pos;
    GLuint vao;
    GLuint vertex_buffer;
    GLuint index_buffer;
    unsigned short number_of_indices; // This probably should be an int
    unsigned int needs_mesh_update : 1;
} ChunkInfo;

// Visible chunks
static list(ChunkInfo) visible_chunks_infos;
static list(Blocks) visible_chunks_blocks;

// TODO invisible chunks
//static int invisible_chunks_count;
//static Blocks *invisible_chunks_blocks;

typedef struct {
    unsigned int is_valid : 1;
    unsigned int is_visible : 1;
    unsigned int is_compressed : 1;
    unsigned int pos : 30;
} ChunkPointer;

BlockPos chunk_to_block_pos(ChunkPos chunkPos) {
    return (BlockPos) {
            .x = chunkPos.x * CHUNK_SIZE,
            .y = chunkPos.y * CHUNK_SIZE,
            .z = chunkPos.z * CHUNK_SIZE
    };
}

ChunkPos chunk_pos_for_block(BlockPos blockPos) {
    return (ChunkPos) {
            .x = blockPos.x / CHUNK_SIZE,
            .y = blockPos.y / CHUNK_SIZE,
            .z = blockPos.z / CHUNK_SIZE
    };
}

static unsigned int chunk_index_offset_x = 0;
static unsigned int chunk_index_offset_z = 0;
static ChunkPointer chunk_index[HORIZONTAL_CHUNKS][VERTICAL_CHUNKS][HORIZONTAL_CHUNKS];

static inline ChunkPointer *access_chunk_index(ChunkPos pos) {
    return &chunk_index[pos.x % HORIZONTAL_CHUNKS][pos.y][pos.z % HORIZONTAL_CHUNKS];
}

static inline int can_acces_chunk(ChunkPos pos) {
    return pos.x >= chunk_index_offset_x
           && pos.x < chunk_index_offset_x + HORIZONTAL_CHUNKS
           && pos.y < VERTICAL_CHUNKS
           && pos.z >= chunk_index_offset_z
           && pos.z < chunk_index_offset_z + HORIZONTAL_CHUNKS;
}

void center_world_at(unsigned int player_x, unsigned int player_z, unsigned int range) {
    unsigned int half_size = HORIZONTAL_CHUNKS * CHUNK_SIZE / 2;
    unsigned int current_center_x = chunk_index_offset_x * CHUNK_SIZE + half_size;
    unsigned int current_center_z = chunk_index_offset_z * CHUNK_SIZE + half_size;

    int dx = player_x - current_center_x;
    int dz = player_z - current_center_z;

    if (dx * dx + dz * dz > range * range) {
        dx /= CHUNK_SIZE;
        dz /= CHUNK_SIZE;
        // only think about the new chunks that are added
        // -d..-1 or size..size+d
        unsigned int offset_x = dx >= 0 ? chunk_index_offset_x + HORIZONTAL_CHUNKS : chunk_index_offset_x + dx;
        unsigned int offset_z = dz >= 0 ? chunk_index_offset_z + HORIZONTAL_CHUNKS : chunk_index_offset_z + dz;

        chunk_index_offset_x += dx;
        chunk_index_offset_z += dz;

        unsigned int end_x = (unsigned int) abs(dx);
        if (end_x > HORIZONTAL_CHUNKS)
            end_x = HORIZONTAL_CHUNKS;
        for (unsigned int x = 0; x < end_x; ++x) {
            for (unsigned int y = 0; y < VERTICAL_CHUNKS; ++y) {
                for (unsigned int z = 0; z < HORIZONTAL_CHUNKS; ++z) {
                    make_visible_chunk((ChunkPos) {offset_x + x, y, z});
                }
            }
        }

        unsigned int end_z = (unsigned int) abs(dz);
        if (end_z > HORIZONTAL_CHUNKS)
            end_z = HORIZONTAL_CHUNKS;
        for (unsigned int z = 0; z < end_z; ++z) {
            for (unsigned int x = 0; x < HORIZONTAL_CHUNKS; ++x) {
                for (unsigned int y = 0; y < VERTICAL_CHUNKS; ++y) {
                    make_visible_chunk((ChunkPos) {x, y, offset_z + z});
                }
            }
        }
    }
}

struct osn_context *osn;

void setup_world_generator() {
    open_simplex_noise(123, &osn);
}

void destroy_world_generator() {
    open_simplex_noise_free(osn);
}

//
// World generator
//
static int octaves = 5; // 8
static float lacunarity = 2.0f; // 2
static float gain = 0.5f; // 0.5
static float a = 2; // 0.5
static float seed = 14479.0f;
static float amplitude = 1.0f; // 1
static float frequency = 0.005f; // 0.02

static float density_at(float x, float y, float z) {
    float ampl = amplitude;
    float freq = frequency;
    float sum = 0;
    double height = open_simplex_noise2(osn, x / 1024, z / 1024.0f) + 0.7;
    height *= 10;

    for (int i = 0; i < octaves; ++i) {
        sum += ampl * (open_simplex_noise4(osn, x * freq, y * freq, z * freq, seed * freq) -
                       a * (y - 128.0f - height) / 128.0f);
        ampl *= gain;
        freq *= lacunarity;
    }
    return sum;
}

void make_visible_chunk(ChunkPos position) {
    BlockPos blockPos = chunk_to_block_pos(position);

    Blocks blocks = empty_blocks;
    for (int ix = 0; ix < CHUNK_SIZE; ++ix) {
        for (int iy = 0; iy < CHUNK_SIZE; ++iy) {
            for (int iz = 0; iz < CHUNK_SIZE; ++iz) {
                int block_x = blockPos.x + ix;
                int block_y = blockPos.y + iy;
                int block_z = blockPos.z + iz;

                float density = density_at(block_x, block_y, block_z);
                Block block = 0;
                if (density > 0.15) {
                    block = STONE_BLOCK;
                } else if (density > 0) {
                    if (density < 0.04 && block_y > 126 && block_y < 130) {
                        block = SAND_BLOCK;
                    } else {
                        block = GRASS_BLOCK;
                    }
                } else if (block_y > 127) {
                    block = AIR_BLOCK;
                } else {
                    block = WATER_BLOCK;
                }

                blocks.data[ix][iy][iz] = block;
            }
        }
    }

    ChunkPointer *cp = access_chunk_index(position);
    *cp = (ChunkPointer) {.is_valid = 1, .is_visible = 1, .pos = visible_chunks_blocks.length};

    list_add(visible_chunks_blocks, blocks);
    list_add(visible_chunks_infos, ((ChunkInfo) {.pos = position, .needs_mesh_update = 1}));
}

//
//
//

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

static list(ChunkVertex) vertex_buffer;
static list(ChunkIndex) index_buffer;

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
    int num_tiles = 4;
    float tile_size = 1.0f / num_tiles;

    v0.u = v1.u = 0;
    v2.u = v3.u = tile_size;

    v0.v = v3.v = tile_size;
    v1.v = v2.v = 0;

    int x_offset;
    int y_offset;
    switch (block) {
        case GRASS_BLOCK: {
            x_offset = 0;
            y_offset = 0;
            break;
        }
        case STONE_BLOCK: {
            x_offset = 1;
            y_offset = 0;
            break;
        }
        case WATER_BLOCK: {
            x_offset = 2;
            y_offset = 0;
            break;
        }
        case SAND_BLOCK: {
            x_offset = 3;
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
    {
        ChunkIndex i = vertex_buffer.length;

        list_ensure_free_capacity(index_buffer,6);
        list_add_unsafe(index_buffer, i);
        list_add_unsafe(index_buffer, i + 1);
        list_add_unsafe(index_buffer, i + 2);
        list_add_unsafe(index_buffer, i);
        list_add_unsafe(index_buffer, i + 2);
        list_add_unsafe(index_buffer, i + 3);

        list_ensure_free_capacity(vertex_buffer, 4);
        list_add_unsafe(vertex_buffer, v0);
        list_add_unsafe(vertex_buffer, v1);
        list_add_unsafe(vertex_buffer, v2);
        list_add_unsafe(vertex_buffer, v3);
    }

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

Block block_at(BlockPos position) {
    ChunkPos chunkPos = chunk_pos_for_block(position);

    if (can_acces_chunk(chunkPos)) {
        ChunkPointer *p = access_chunk_index(chunkPos);
        if (p && p->is_valid && p->is_visible) {
            Blocks *blocks = &visible_chunks_blocks.data[p->pos];
            return blocks->data
            [position.x - chunkPos.x * CHUNK_SIZE]
            [position.y - chunkPos.y * CHUNK_SIZE]
            [position.z - chunkPos.z * CHUNK_SIZE];
        }
    }
    return 0;
}

static void update_mesh(int index, ChunkInfo *info) {
    // TODO limit update time per frame

    Blocks *blocks = &visible_chunks_blocks.data[index];

    vertex_buffer.length = 0;
    index_buffer.length = 0;

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
    ChunkPos next_x_pos = info->pos;
    ++next_x_pos.x;
    if (can_acces_chunk(next_x_pos)) {
        ChunkPointer *next_x_chunk = access_chunk_index(next_x_pos);
        if (next_x_chunk->is_valid && next_x_chunk->is_visible)
            next_x_blocks = &visible_chunks_blocks.data[next_x_chunk->pos];
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
    ChunkPos next_y_pos = info->pos;
    ++next_y_pos.y;
    Blocks *next_y_blocks = NULL;
    if (can_acces_chunk(next_y_pos)) {
        ChunkPointer *next_y_chunk = access_chunk_index(next_y_pos);
        if (next_y_chunk->is_valid && next_y_chunk->is_visible)
            next_y_blocks = &visible_chunks_blocks.data[next_y_chunk->pos];
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

    ChunkPos next_z_pos = info->pos;
    ++next_z_pos.z;
    Blocks *next_z_blocks = NULL;
    if (can_acces_chunk(next_z_pos)) {
        ChunkPointer *next_z_chunk = access_chunk_index(next_z_pos);
        if (next_z_chunk->is_valid && next_z_chunk->is_visible)
            next_z_blocks = &visible_chunks_blocks.data[next_z_chunk->pos];
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
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer.length * sizeof(ChunkVertex), vertex_buffer.data, GL_STATIC_DRAW);

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer.length * sizeof(ChunkIndex), index_buffer.data, GL_STATIC_DRAW);

    info->number_of_indices = index_buffer.length;
    info->needs_mesh_update = 0;
}

void render_chunks(float projection_view[]) {
    // TODO setup shader
    glUseProgram(shader1);

    float mvp[MAT4_SIZE];
    float model[MAT4_SIZE];
    float pos[VEC3_SIZE];

    GLuint mvp_location = glGetUniformLocation(shader1, "MVP");

    for (int i = 0; i < visible_chunks_infos.length; ++i) {
        ChunkInfo *info = &visible_chunks_infos.data[i];

        if (is_culled(info))
            continue;

        if (info->needs_mesh_update)
            update_mesh(i, info);

        mat4_identity(model);
        mat4_translation(model, model,
                         vec3(pos, (int) info->pos.x * CHUNK_SIZE, (int) info->pos.y * CHUNK_SIZE,
                              (int) info->pos.z * CHUNK_SIZE));
        mat4_multiply(mvp, projection_view, model);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *) mvp);

        glBindVertexArray(info->vao);
        glDrawElements(GL_TRIANGLES, info->number_of_indices, GL_UNSIGNED_SHORT, 0);
    }
}


