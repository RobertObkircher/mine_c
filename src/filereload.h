//
// Created by robert on 1/3/19.
//

#ifndef MINE_C_FILERELOAD_H
#define MINE_C_FILERELOAD_H

#endif //MINE_C_FILERELOAD_H

#define FILERELOAD_MAX_WATCHED_FILES 100

void init_filereload();

void close_filereload();

void listen_for_file_changes(const char *directory_name, const char *filename,
                             void (*callback)(const char *directory_name, const char *filename));

int update_filereload();
