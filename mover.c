#include "mover.h"
#include "globel.c"


// Function to create the "Processed" directory if it does not exist
int create_processed_directory()
{
    const char *dir = "./home/Processed";
    struct stat st = {0};

    if (stat(dir, &st) == -1)
    {
        if (mkdir(dir, 0700) == -1)
        {
            perror("Failed to create Processed directory");
            return -1;
        }
        printf("Directory '%s' created successfully.\n", dir);
    }
    else
    {
        printf("Directory '%s' already exists.\n", dir);
    }

    return 0; 
}
// Define and initialize the mutex here

void initialize_fifo_mutex()
{
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&fifo_mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);
}

// Function to move a file to the "Processed" directory
int move_file_to_processed(const char *filename)
{
    const char *processed_dir = "./home/Processed/";

    // Build the destination path
    char destination[MAX_FILENAME_LENGTH];
    snprintf(destination, sizeof(destination), "%s%s", processed_dir, filename);

    // Move the file using the rename function (this can fail if the source and destination are on different file systems)
    if (rename(filename, destination) == -1)
    {
        perror("Failed to move file");
        return -1;
    }

    printf("File %s moved to Processed directory.\n", filename);
    return 0;
}

// Mover thread function that moves files after processing
void *mover_thread(void *arg)
{
    // Create the "Processed" directory if it doesn't exist
    // Open the FIFO for reading
    int fifo_fd = open(FIFO_PATH_MOVE, O_RDONLY);
    if (fifo_fd < 0)
    {
        perror("Error opening FIFO for reading");
        return NULL;
    }

    char buffer[MAX_FILENAME_LENGTH];
    while (1)
    {
        // Lock the mutex to ensure mutual exclusion with the calculator thread
        pthread_mutex_lock(&fifo_mutex);

        ssize_t bytesRead = read(fifo_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            char *filename = strtok(buffer, "\n");

            if (filename != NULL)
            {
                printf("Mover thread is moving file: %s\n", filename);
                move_file_to_processed(filename);
            }
        }

        // Unlock the mutex after reading the FIFO
        pthread_mutex_unlock(&fifo_mutex);
    }

    close(fifo_fd);
    return NULL;
}
