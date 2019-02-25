#ifndef MINE_C_CHUNKS_H
#define MINE_C_CHUNKS_H

#define CHUNK_SIZE 8
#define HORIZONTAL_CHUNKS 5
#define VERTICAL_CHUNKS 32

typedef unsigned char Block;

typedef enum {
    AIR_BLOCK,
    GRASS_BLOCK,
    WATER_BLOCK,
    STONE_BLOCK,
    SAND_BLOCK,
    ICE_BLOCK,
    BEDROCK_BLOCK,
    WOOD_BLOCK,
    BRICKS_BLOCK,
    PUMPKIN_BLOCK,
    LEAVES_BLOCK,
} BlockType;

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int z;
} ChunkPos;

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int z;
} BlockPos;

void setup_world_generator();
void destroy_world_generator();

void render_chunks(float projection_view[]);

void make_visible_chunk(ChunkPos position);

Block block_at(BlockPos position);

void center_world_at(unsigned int player_x, unsigned int player_z, unsigned int range);

#endif //MINE_C_CHUNKS_H
