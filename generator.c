#include <sys/types.h>
#include "generator.h"




// Function to initialize the semaphore and shared memory
void init_semaphore()
{
    key_t key = ftok("file_gen_key", 65);      // Generate a unique key
    sem_id = semget(key, 1, 0666 | IPC_CREAT); // Create or get semaphore
    if (sem_id == -1)
    {
        perror("semget failed");
        exit(1);
    }

    // Initialize the semaphore value (1 for binary semaphore)
    union semun sem_union;
    sem_union.val = 1; // Initial value (mutex-like behavior)
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
        perror("semctl failed");
        exit(1);
    }

    // Allocate shared memory for the file count
    key_t shm_key = ftok("file_gen_shm_key", 66); // Unique key for shared memory
    int shm_id = shmget(shm_key, sizeof(struct MEMORY), 0666 | IPC_CREAT);
    if (shm_id == -1)
    {
        perror("shmget failed");
        exit(1);
    }

    // Attach to shared memory
    shared_memory = (struct MEMORY *)shmat(shm_id, NULL, 0);
    if (shared_memory == (struct MEMORY *)-1)
    {
        perror("shmat failed");
        exit(1);
    }

    // Initialize file_count in shared memory
    shared_memory->file_count = 0;
    shared_memory->num_calculators = 0;
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_memory->file_mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);
}

// Function to generate a random float in a given range
float get_random_value(float min_value, float max_value)
{
    return ((float)rand() / RAND_MAX) * (max_value - min_value) + min_value;
}


void generate_csv_file(int generator_id)
{
    // Ensure the "home" directory exists
    struct stat st = {0};
    if (stat("home", &st) == -1)
    {
        mkdir("home", 0777); // Create the "home" directory with full permissions
    }
     //printf("Current time: %s", ctime(&(time_t){time(NULL)}));  // Print current time in one line
    // Protect shared resource (file_count) with semaphore
    semop(sem_id, &acquire, 1); // Wait for access to the file count
    //printf("Current time: %s", ctime(&(time_t){time(NULL)}));  // Print current time in one line


    // Get the current file count from shared memory
    int file_count = shared_memory->file_count;

    // Generate a unique file name in the "home" directory
    char filename[100];
    snprintf(filename, sizeof(filename), "home/%d.csv", file_count);

    // Open the new file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Error creating file");
        semop(sem_id, &release, 1); // Release the semaphore before returning
        return;
    }

    // Increment the file count in shared memory
    shared_memory->file_count = file_count + 1;

    // Release semaphore after updating the shared resource
    semop(sem_id, &release, 1); // Release access to the file count
    //printf("Current time: %s", ctime(&(time_t){time(NULL)}));  // Print current time in one line


    // Generate a random number of rows and columns within the global range
    int rows = rand() % (max_rows - min_rows + 1) + min_rows;
    int cols = rand() % (max_cols - min_cols + 1) + min_cols;

    // Write CSV headers
    for (int col = 0; col < cols; col++)
    {
        fprintf(file, "Col%d", col + 1);
        if (col < cols - 1)
            fprintf(file, ",");
    }
    fprintf(file, "\n");

    int deleted_count = 0;
    int number_of_delete = ceil((rows * cols) * (miss_percentage / (double)100.0));
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            // Decide randomly if this value should be deleted
            if (deleted_count < number_of_delete && rand() % (rows * cols) < number_of_delete)
            {
                deleted_count++;
                fprintf(file, " ");
            }
            else
            {
                float random_value = get_random_value(min_value, max_value); // Get random float value
                fprintf(file, "%.2f", random_value);                         // Write value directly to file
            }

            if (j < cols - 1)
            {
                fprintf(file, ","); // Add comma if it's not the last column
            }
        }
        fprintf(file, "\n"); // New line after each row
    }

    fclose(file);
    printf("Generator %d created file: %s with %d rows and %d columns\n", generator_id, filename, rows, cols);
    // // Open the FIFO for writing
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd < 0)
    {
        perror("Error opening FIFO");
        return;
    }
 
    // Write the file name to the FIFO
    if (write(fifo_fd, filename, strlen(filename)) < 0)
    {
        perror("Error writing to FIFO");
    }
    write(fifo_fd, "\n", 1);

    // Close the FIFO
    close(fifo_fd);
}

// Thread function for each generator
void *file_generator(void *arg)
{
    GeneratorParams *params = (GeneratorParams *)arg;

    while (1)
    {
        generate_csv_file(params->generator_id);
        int wait_time = rand() % (params->max_time - params->min_time + 1) + params->min_time;
        sleep(wait_time);
    }

    return NULL;
}
