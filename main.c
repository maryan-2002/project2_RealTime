#include "header.h"
#include "globel.c"
#include "generator.h"

// Global variables for configuration
int min_rows = DEFAULT_MIN_ROWS;
int max_rows = DEFAULT_MAX_ROWS;
int min_cols = DEFAULT_MIN_COLS;
int max_cols = DEFAULT_MAX_COLS;
int min_value = DEFAULT_MIN_VALUE;
int max_value = DEFAULT_MAX_VALUE;
int miss_percentage = DEFAULT_MISS_PERCENTAGE;

struct sembuf acquire = {0, -1, SEM_UNDO},  release = {0,  1, SEM_UNDO};  
// Shared semaphore ID
int sem_id;
struct MEMORY *shared_memory; // Shared memory pointer


int main(int argc, char *argv[]) {
    int num_generators = DEFAULT_GENERATORS;
    int min_time = DEFAULT_MIN_TIME;
    int max_time = DEFAULT_MAX_TIME;

    printf("number of arg =%d", argc);
    // Parse user input for number of generators and time range
    if (argc > 1) num_generators = atoi(argv[1]);
    if (argc > 2) min_time = atoi(argv[2]);
    if (argc > 3) max_time = atoi(argv[3]);

    // Parse user input for the global CSV parameters
    if (argc > 4) min_rows = atoi(argv[4]);
    if (argc > 5) max_rows = atoi(argv[5]);
    if (argc > 6) min_cols = atoi(argv[6]);
    if (argc > 7) max_cols = atoi(argv[7]);
    if (argc > 8) min_value = atof(argv[8]);
    if (argc > 9) max_value = atof(argv[9]);
    if (argc > 10) miss_percentage = atoi(argv[10]);

    if (min_time > max_time) {
        fprintf(stderr, "Invalid time range: min_time should be <= max_time\n");
        return EXIT_FAILURE;
    }

    printf("Starting %d file generators with time range [%d, %d] seconds.\n", num_generators, min_time, max_time);
    printf("Global settings: %d rows, %d cols, value range [%.2d, %.d], miss percentage: %d%%\n",
           max_rows, max_cols, min_value, max_value, miss_percentage);

    // Seed random number generator
    srand(time(NULL));

    // Create threads for each file generator
    init_semaphore();
    pthread_t threads[num_generators];
    GeneratorParams params[num_generators];

    for (int i = 0; i < num_generators; i++) {      
        params[i].generator_id = i + 1;
        params[i].min_time = min_time;
        params[i].max_time = max_time;

        if (pthread_create(&threads[i], NULL, file_generator, &params[i]) != 0) {
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    // Join threads (optional; can be omitted if continuous operation is desired)
    for (int i = 0; i < num_generators; i++) {
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}