#include "header.h"
#include "calculator.h"


// Function to calculate column averages for a given CSV file
void calculate_csv_file(const char *filename, int calculator_id) {

     struct MEMORYcalculator *newValue = malloc(sizeof(struct MEMORYcalculator));
    if (!newValue) {
        perror("Failed to allocate memory for MEMORYcalculator");
        return;
    }
    newValue->calculator_id = calculator_id;

    // Allocate and copy the filename
    if (!newValue->file_number) {
        perror("Failed to allocate memory for file_number");
        free(newValue);
        return;
    }
    strcpy(newValue->file_number, filename);

    // Initialize other fields
    newValue->num_rows = 0;
    newValue->column_averages = malloc(max_cols * sizeof(double));
    if (!newValue->column_averages) {
        perror("Failed to allocate memory for column_averages");
        free(newValue->file_number);
        free(newValue);
        return;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }
    int column_sums[] = {0};    
    int column_counts[] = {0}; 
    int column_total = 0;       
    char line[1024];              


    // Read file line by line
    // while (fgets(line, sizeof(line), file)) {
    //     newValue->num_rows++; // Increment the row count

    //     column_total = 0; // Reset column count for the current row
    //     char *token = strtok(line, ","); // Split by comma
    //     while (token) {
    //         // Process each value
    //         if (*token != '\n' && *token != '\r') { // Ignore newlines
    //             int value;
    //             if (sscanf(token, "%d", &value) == 1) {
    //                 column_sums[column_total] += value;
    //                 column_counts[column_total]++;
    //             }
    //         }
    //         token = strtok(NULL, ",");
    //         column_total++;
    //     }
    // }
    fclose(file);

    // // Allocate memory for column averages
    // newValue->column_averages = malloc(column_total * sizeof(double));
    // if (!newValue->column_averages) {
    //     perror("Failed to allocate memory for column averages");
    //     return;
    // }

    // // Calculate averages
    // for (int i = 0; i < column_total; i++) {
    //     if (column_counts[i] > 0) {
    //         newValue->column_averages[i] = (double)column_sums[i] / column_counts[i];
    //     } else {
    //         newValue->column_averages[i] = 0.0; // No valid values
    //     }
    // }

    // printf("Calculator ID: %d\n", newValue->calculator_id);
    // printf("Processed File: %s\n", newValue->file_number);
    // printf("Number of Rows: %d\n", newValue->num_rows);
    // printf("Column Averages:\n");
    // for (int i = 0; i < column_total; i++) {
    //     printf("  Column %d: %.2f\n", i + 1, newValue->column_averages[i]);
    // }
}



// Example calculator thread function
// Calculator thread function
void *calculator_thread(void *arg) {
    CalculatorParams *params = (CalculatorParams *)arg;
    printf("Calculator thread %d started, processing up to %d columns.\n",
           params->calculator_id, params->max_cols);

    // Loop until all files are processed
    while (1) {
        // Lock the mutex to check and update the file serial number
        pthread_mutex_lock(&shared_memory->file_mutex);

        if (shared_memory->file_count > shared_memory->num_calculators ) {
            int file_serial = shared_memory->num_calculators ;
            shared_memory->num_calculators ++; // Decrement the file count for the next file

            // Unlock the mutex after accessing the shared resource
            pthread_mutex_unlock(&shared_memory->file_mutex);

            // Create the filename using the serial number
            char filename[256];
            snprintf(filename, sizeof(filename), "%d.csv", file_serial);
            sleep(3);
            printf("Calculator thread %d started, processing up to %d files.\n",
            params->calculator_id, file_serial);

            // Process the CSV file (call the actual function to calculate averages, etc.)
            calculate_csv_file(filename,params->calculator_id);
        } else {
            // No files left to process, unlock the mutex and exit
            pthread_mutex_unlock(&shared_memory->file_mutex);
        }
    }

    return NULL;
}

// int main(int argc, char *argv[]) {
//     if (argc < 2) {
//         fprintf(stderr, "Usage: %s <csv_file>\n", argv[0]);
//         return EXIT_FAILURE;
//     }

//     // Attach to shared memory
//     key_t shm_key = ftok("shared_memory_key", 1);
//     int shm_id = shmget(shm_key, sizeof(struct MEMORY), 0666);
//     if (shm_id == -1) {
//         perror("Shared memory access failed");
//         return EXIT_FAILURE;
//     }
//     struct MEMORY *shared_memory = (struct MEMORY *)shmat(shm_id, NULL, 0);
//     if (shared_memory == (void *)-1) {
//         perror("Shared memory attach failed");
//         return EXIT_FAILURE;
//     }

//     // Access semaphore
//     key_t sem_key = ftok("semaphore_key", 1);
//     int sem_id = semget(sem_key, 1, 0666);
//     if (sem_id == -1) {
//         perror("Semaphore access failed");
//         return EXIT_FAILURE;
//     }

//     // Process the CSV file
//     calculate_csv_file(argv[1], shared_memory, sem_id);

//     // Detach from shared memory
//     shmdt(shared_memory);

//     return EXIT_SUCCESS;
// }
