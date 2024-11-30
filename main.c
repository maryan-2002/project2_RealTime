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
int delete_value = MAX_TIME_VALUE;

int num_generators = DEFAULT_GENERATORS;
int num_calculators = DEFAULT_GENERATORS;
int num_movers = 10; // Default number of movers is 10
int num_inspectors = 10;
int min_time = DEFAULT_MIN_TIME;
int max_time = DEFAULT_MAX_TIME;

int num_type1=1;
int num_type2=1;
int num_type3=1;
int age_limit= 86400;

procees_th = 100;
unprocees_th = 100;
backup_th = 100;
delete_th = 100;
runtime_th = 60;

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
            else if (strcmp(key, "num_type1") == 0)
            {
                num_type1 = atoi(value);
            }
            else if (strcmp(key, "num_type2") == 0)
            {
                num_type2 = atoi(value);
            }
            else if (strcmp(key, "num_type3") == 0)
            {
                num_type3 = atoi(value);
            }
            else if (strcmp(key, "age_limit") == 0)
            {
                age_limit = atoi(value);
            }
            else if (strcmp(key, "procees_th") == 0)
            {
                procees_th = atoi(value);
            }
            else if (strcmp(key, "unprocees_th") == 0)
            {
                unprocees_th = atoi(value);
            }
            else if (strcmp(key, "backup_th") == 0)
            {
                backup_th = atoi(value);
            }
            else if (strcmp(key, "delete_th") == 0)
            {
                delete_th = atoi(value);
            }
            else if (strcmp(key, "runtime_th") == 0)
            {
                runtime_th = atoi(value);
            }
        }
    }

    fclose(file);
    return 0;
}

// Function to kill all threads and exit
void kill_all_and_exit() {
    // Cancel generator threads
    for (int i = 0; i < num_generators; i++) {
        pthread_cancel(generator_threads[i]);
    }

    // Cancel calculator threads
    for (int i = 0; i < num_calculators; i++) {
        pthread_cancel(calculator_threads[i]);
    }

    // Cancel mover threads
    for (int i = 0; i < num_movers; i++) {
        pthread_cancel(mover_threads[i]);
    }

    printf("All threads terminated forcefully. Exiting program.\n");

    // Free dynamically allocated memory
    free(generator_threads);
    free(calculator_threads);
    free(mover_threads);

    exit(EXIT_SUCCESS);
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

    create_processed_directory();
    // Create threads for each CSV file mover
    pthread_t mover_threads[num_movers];
    for (int i = 0; i < num_movers; i++)
    {
        if (pthread_create(&mover_threads[i], NULL, mover_thread, NULL) != 0)
        {
            perror("Failed to create mover thread");
            return EXIT_FAILURE;
        }
    }

    pthread_t inspector_threads[num_inspectors];
    struct SharedData shared_data = {0};

    // for (int i = 0; i < num_inspectors; i++)
    // {
    //     if (pthread_create(&inspector_threads[i], NULL, inspector_thread, &shared_data) != 0)
    //     {
    //         perror("Failed to create inspector thread");
    //         return EXIT_FAILURE;
    //     }
    // }

    // Join generator threads (optional, for continuous operation remove this part)

    // Create threads for inspectors

    pthread_t type1_threads[num_type1];

    pthread_t type2_threads[num_type2];

    pthread_t type3_threads[num_type3];


    for (int i = 0; i < num_type1; i++) {

        if (pthread_create(&type1_threads[i], NULL, inspector_thread_type_1, &age_limit) != 0) {

            perror("Failed to create Type 1 inspector thread");

            return EXIT_FAILURE;

        }

    }

      for (int i = 0; i < num_type2; i++) {

        if (pthread_create(&type2_threads[i], NULL, inspector_thread_type_2, &age_limit) != 0) {

            perror("Failed to create Type 2 inspector thread");

            return EXIT_FAILURE;

        }

    }


    for (int i = 0; i < num_type3; i++) {

        if (pthread_create(&type3_threads[i], NULL, inspector_thread_type_3, &age_limit) != 0) {

            perror("Failed to create Type 3 inspector thread");

            return EXIT_FAILURE;

        }

    }

    
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

    // // Join type1 inspector threads
    // for (int i = 0; i < num_type1; i++)
    // {
    //     pthread_join(type1_threads[i], NULL);
    // }

    // // Join type2 inspector threads
    // for (int i = 0; i < num_type2; i++)
    // {
    //     pthread_join(type2_threads[i], NULL);
    // }

    // // Join movtype3 inspectorer threads
    // for (int i = 0; i < num_type3; i++)
    // {
    //     pthread_join(type3_threads[i], NULL);
    // }

    return EXIT_SUCCESS;
}