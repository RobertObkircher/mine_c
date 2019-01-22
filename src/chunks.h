#ifndef MINE_C_CHUNKS_H
#define MINE_C_CHUNKS_H

#define CHUNK_SIZE 8
#define HORIZONTAL_CHUNKS 5
#define VERTICAL_CHUNKS 32
#define TOTAL_CHUNKS HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS * VERTICAL_CHUNKS

typedef unsigned char Block;

typedef enum {
    AIR_BLOCK,
    GRASS_BLOCK,
    WATER_BLOCK,
    STONE_BLOCK,
    ICE_BLOCK,
    BEDROCK_BLOCK,
    WOOD_BLOCK,
    SAND_BLOCK,
    BRICKS_BLOCK,
    PUMPKIN_BLOCK,
    LEAVES_BLOCK,
} BlockType;

void setup_world_generator();
void destroy_world_generator();

void render_chunks(float projection_view[]);

void make_visible_chunk(int x, int y, int z);

Block block_at(unsigned int x, unsigned int y, unsigned int z);

#endif //MINE_C_CHUNKS_H
