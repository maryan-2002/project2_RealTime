#ifndef MOVER_H
#define MOVER_H

#include "header.h"
#define MAX_FILENAME_LENGTH 256
#define DEFAULT_MOVER_COUNT 10

void *mover_thread(void *arg);

int create_processed_directory();

int move_file_to_processed(const char *filename);
void initialize_fifo_mutex();
#endif // MOVER_H
