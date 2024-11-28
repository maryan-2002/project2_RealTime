#include "header.h"
#include "globel.c"
#include "generator.h"
#include "calculator.h"
#include "mover.h" 

// Global variables for configuration
int min_rows = DEFAULT_MIN_ROWS;
int max_rows = DEFAULT_MAX_ROWS;
int min_cols = DEFAULT_MIN_COLS;
int max_cols = DEFAULT_MAX_COLS;
int min_value = DEFAULT_MIN_VALUE;
int max_value = DEFAULT_MAX_VALUE;
int miss_percentage = DEFAULT_MISS_PERCENTAGE;
pthread_mutex_t fifo_mutex = PTHREAD_MUTEX_INITIALIZER;

struct sembuf acquire = {0, -1, SEM_UNDO}, release = {0, 1, SEM_UNDO};  
// Shared semaphore ID
int sem_id;
struct MEMORY *shared_memory; // Shared memory pointer
struct SharedCalculators calc;

int main(int argc, char *argv[]) {
    
    int num_generators = DEFAULT_GENERATORS;
    int num_calculators = DEFAULT_GENERATORS;
    int num_movers = 10;  // Default number of movers is 10
    int min_time = DEFAULT_MIN_TIME;
    int max_time = DEFAULT_MAX_TIME;

    printf("number of args = %d\n", argc);
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

    // Parse user input for the number of movers (if provided)
    if (argc > 11) num_movers = atoi(argv[11]);

    if (min_time > max_time) {
        fprintf(stderr, "Invalid time range: min_time should be <= max_time\n");
        return EXIT_FAILURE;
    }

    printf("Starting %d file generators with time range [%d, %d] seconds.\n", num_generators, min_time, max_time);
    printf("Global settings: %d rows, %d cols, value range [%.2d, %.d], miss percentage: %d%%\n",
           max_rows, max_cols, min_value, max_value, miss_percentage);
    printf("Starting %d CSV file movers.\n", num_movers);

    // Seed random number generator
    srand(time(NULL));

    if (mkfifo(FIFO_PATH, 0666) < 0) {
        perror("Error creating FIFO");
    }
    if (mkfifo(FIFO_PATH_MOVE, 0666) < 0) {
        perror("Error creating FIFO for movers");
    }



    // Create threads for each file generator
    init_semaphore();
    initialize_fifo_mutex();
    pthread_t generator_threads[num_generators];
    GeneratorParams params[num_generators];

    for (int i = 0; i < num_generators; i++) {      
        params[i].generator_id = i + 1;
        params[i].min_time = min_time;
        params[i].max_time = max_time;

        if (pthread_create(&generator_threads[i], NULL, file_generator, &params[i]) != 0) {
            perror("Failed to create generator thread");
            return EXIT_FAILURE;
        }
    }

    // Create threads for each file calculator
    pthread_t calculator_threads[num_calculators];
    CalculatorParams calculator_params[num_calculators];

    for (int i = 0; i < num_calculators; i++) {
        calculator_params[i].calculator_id = i + 1;
        calculator_params[i].max_cols = max_cols;

        if (pthread_create(&calculator_threads[i], NULL, calculator_thread, &calculator_params[i]) != 0) {
            perror("Failed to create calculator thread");
            return EXIT_FAILURE;
        }
    }

    create_processed_directory();
    // Create threads for each CSV file mover
    pthread_t mover_threads[num_movers];
    for (int i = 0; i < num_movers; i++) {
        if (pthread_create(&mover_threads[i], NULL, mover_thread, NULL) != 0) {
            perror("Failed to create mover thread");
            return EXIT_FAILURE;
        }
    }

    // Join generator threads (optional, for continuous operation remove this part)
    for (int i = 0; i < num_generators; i++) {
        pthread_join(generator_threads[i], NULL);
    }

    // Join calculator threads
    for (int i = 0; i < num_calculators; i++) {
        pthread_join(calculator_threads[i], NULL);
    }

    // Join mover threads
    for (int i = 0; i < num_movers; i++) {
        pthread_join(mover_threads[i], NULL);
    }

    return EXIT_SUCCESS;
}
