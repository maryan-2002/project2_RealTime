
#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <pthread.h>
#include<sys/shm.h>
#include <errno.h>
#include <GL/glut.h>
#include <math.h>



// Declare the variables as extern in the header file
extern int min_rows;
extern int max_rows;
extern int min_cols;
extern int max_cols;
extern int min_value;
extern int max_value;
extern int miss_percentage;

// Data structure to hold generator parameters
typedef struct {
    int generator_id;
    int min_time;
    int max_time;
} GeneratorParams;


// Shared memory structure for the file count (serial number)
struct MEMORY {
    int file_count; // Shared file count
};

// Semaphore operations
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

extern struct sembuf acquire;
extern struct sembuf release;
// Shared semaphore ID
extern int sem_id;
extern struct MEMORY *shared_memory; // Shared memory pointer



#endif