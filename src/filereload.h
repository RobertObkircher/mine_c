#ifndef MINE_C_FILERELOAD_H
#define MINE_C_FILERELOAD_H

#endif //MINE_C_FILERELOAD_H

#define FILERELOAD_MAX_WATCHED_FILES 100

void init_filereload();

void close_filereload();

void listen_for_file_changes(const char *directory_name, const char *filename,
                             void (*callback)(char *path));

int update_filereload();
