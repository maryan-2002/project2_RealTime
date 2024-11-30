#include "header.h"
#include "globel.c"
#include "generator.h"
#include "calculator.h"
#include "mover.h"
#include "inspector.h"

// Global variables for configuration
int min_rows = DEFAULT_MIN_ROWS;
int max_rows = DEFAULT_MAX_ROWS;
int min_cols = DEFAULT_MIN_COLS;
int max_cols = DEFAULT_MAX_COLS;
int min_value = DEFAULT_MIN_VALUE;
int max_value = DEFAULT_MAX_VALUE;
int miss_percentage = DEFAULT_MISS_PERCENTAGE;
int unprocessed_value = MAX_TIME_VALUE;
int backup_value = MAX_BACKUP;
int delete_value = MAX_BACKUP;

int num_generators = DEFAULT_GENERATORS;
int num_calculators = DEFAULT_GENERATORS;
int num_movers = 10; // Default number of movers is 10
int num_inspectors = 10;
int min_time = DEFAULT_MIN_TIME;
int max_time = DEFAULT_MAX_TIME;

pthread_mutex_t fifo_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shared_mutex_inspector = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shared_mutex_backup = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shared_mutex_deleate = PTHREAD_MUTEX_INITIALIZER;
struct sembuf acquire = {0, -1, SEM_UNDO}, release = {0, 1, SEM_UNDO};
// Shared semaphore ID
int sem_id;
struct MEMORY *shared_memory; // Shared memory pointer
struct SharedCalculators calc;

// Function to read arguments from the file
int read_arguments_from_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening arguments file");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        // Remove newline characters from the line
        line[strcspn(line, "\n")] = 0;

        // Split by the colon ":"
        char *key = strtok(line, ":");
        char *value = strtok(NULL, ":");

        if (key && value)
        {
            // Remove any leading or trailing spaces from key and value
            while (*key == ' ')
                key++;
            while (*value == ' ')
                value++;

            // Check the key and assign the corresponding value
            if (strcmp(key, "min_rows") == 0)
            {
                min_rows = atoi(value);
            }
            else if (strcmp(key, "max_rows") == 0)
            {
                max_rows = atoi(value);
            }
            else if (strcmp(key, "min_cols") == 0)
            {
                min_cols = atoi(value);
            }
            else if (strcmp(key, "max_cols") == 0)
            {
                max_cols = atoi(value);
            }
            else if (strcmp(key, "min_value") == 0)
            {
                min_value = atof(value);
            }
            else if (strcmp(key, "max_value") == 0)
            {
                max_value = atof(value);
            }
            else if (strcmp(key, "miss_percentage") == 0)
            {
                miss_percentage = atoi(value);
            }
            else if (strcmp(key, "num_generators") == 0)
            {
                num_generators = atoi(value);
            }
            else if (strcmp(key, "num_movers") == 0)
            {
                num_movers = atoi(value);
            }
            else if (strcmp(key, "min_time") == 0)
            {
                min_time = atoi(value);
            }
            else if (strcmp(key, "max_time") == 0)
            {
                max_time = atoi(value);
            }
        }
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[])
{
     if (read_arguments_from_file("arguments.txt") != 0) {
        return EXIT_FAILURE;
    }
   
    printf("Starting %d file generators with time range [%d, %d] seconds.\n", num_generators, min_time, max_time);
    printf("Global settings: %d rows, %d cols, value range [%.2d, %.d], miss percentage: %d%%\n",
           max_rows, max_cols, min_value, max_value, miss_percentage);
    printf("Starting %d CSV file movers.\n", num_movers);

    // Seed random number generator
    srand(time(NULL));

    if (mkfifo(FIFO_PATH, 0666) < 0)
    {
        perror("Error creating FIFO");
    }
    if (mkfifo(FIFO_PATH_MOVE, 0666) < 0)
    {
        perror("Error creating FIFO for movers");
    }

    // Create threads for each file generator
    init_semaphore();
    initialize_fifo_mutex();
    pthread_t generator_threads[num_generators];
    GeneratorParams params[num_generators];
    for (int i = 0; i < num_generators; i++)
    {
       
        
        params[i].generator_id = i + 1;
        params[i].min_time = min_time;
        params[i].max_time = max_time;

        if (pthread_create(&generator_threads[i], NULL, file_generator, &params[i]) != 0)
        {
            perror("Failed to create generator thread");
            return EXIT_FAILURE;
        }
    }

    // // Create threads for each file calculator
    pthread_t calculator_threads[num_calculators];
    CalculatorParams calculator_params[num_calculators];

    for (int i = 0; i < num_calculators; i++)
    {
        calculator_params[i].calculator_id = i + 1;
        calculator_params[i].max_cols = max_cols;

        if (pthread_create(&calculator_threads[i], NULL, calculator_thread, &calculator_params[i]) != 0)
        {
            perror("Failed to create calculator thread");
            return EXIT_FAILURE;
        }
    }

    // create_processed_directory();
    // // Create threads for each CSV file mover
    // pthread_t mover_threads[num_movers];
    // for (int i = 0; i < num_movers; i++)
    // {
    //     if (pthread_create(&mover_threads[i], NULL, mover_thread, NULL) != 0)
    //     {
    //         perror("Failed to create mover thread");
    //         return EXIT_FAILURE;
    //     }
    // }

    pthread_t inspector_threads[num_inspectors];
    struct SharedData shared_data = {0};

    for (int i = 0; i < num_inspectors; i++)
    {
        if (pthread_create(&inspector_threads[i], NULL, inspector_thread, &shared_data) != 0)
        {
            perror("Failed to create inspector thread");
            return EXIT_FAILURE;
        }
    }

    // Join generator threads (optional, for continuous operation remove this part)
    for (int i = 0; i < num_generators; i++)
    {
        pthread_join(generator_threads[i], NULL);
    }

    // // Join calculator threads
    // for (int i = 0; i < num_calculators; i++)
    // {
    //     pthread_join(calculator_threads[i], NULL);
    // }

    // // Join mover threads
    // for (int i = 0; i < num_movers; i++)
    // {
    //     pthread_join(mover_threads[i], NULL);
    // }

    return EXIT_SUCCESS;
}
