
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
extern int unprocessed_value;
extern int  delete_value;
extern int backup_value;

// Data structure to hold generator parameters
typedef struct {
    int generator_id;
    int min_time;
    int max_time;
} GeneratorParams;


struct MEMORY {
    int file_count;           // Shared file count (number of processed files)
    int num_calculators;      // Number of file calculators (threads)
    int unprocessed_count;
    int backup_count;
    int deleted_count;
    pthread_mutex_t file_mutex;    // Mutex to synchronize file access
};

typedef struct {
    int calculator_id;
    int max_cols;
} CalculatorParams;

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
extern struct SharedCalculators calc;
extern pthread_mutex_t fifo_mutex; 
extern pthread_mutex_t shared_mutex_inspector ;
extern pthread_mutex_t shared_mutex_backup;
extern pthread_mutex_t shared_mutex_deleate ;


struct MEMORYcalculator {
    int calculator_id;
    char *file_number;
    int num_rows;
    double *column_averages;
    struct MEMORYcalculator *next;  // Pointer to the next node
};

// Shared array of MEMORYcalculator with mutex
struct SharedCalculators {
    struct MEMORYcalculator *calculators; // Array of MEMORYcalculator
    pthread_mutex_t mutex;               // Mutex for synchronization
};

#endif