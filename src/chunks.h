#ifndef MINE_C_CHUNKS_H
#define MINE_C_CHUNKS_H

#define CHUNK_SIZE 8
#define HORIZONTAL_CHUNKS 5
#define VERTICAL_CHUNKS 3
#define TOTAL_CHUNKS HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS * VERTICAL_CHUNKS

void render_chunks(float projection_view[]);

void make_visible_chunk(int x, int y, int z);

#endif //MINE_C_CHUNKS_H
