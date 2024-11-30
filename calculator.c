#include "header.h"
#include "calculator.h"
#include "globel.c"

void calculate_csv_file(const char *filename, int calculator_id)
{
    struct MEMORYcalculator *newValue = malloc(sizeof(struct MEMORYcalculator));
    if (!newValue)
    {
        perror("Failed to allocate memory for MEMORYcalculator");
        return;
    }

    newValue->calculator_id = calculator_id;
    newValue->file_number = malloc(strlen(filename) + 1);
    if (!newValue->file_number)
    {
        perror("Failed to allocate memory for file_number");
        free(newValue);
        return;
    }
    strcpy(newValue->file_number, filename);

    newValue->num_rows = 0;
    newValue->column_averages = calloc(max_cols, sizeof(double));
    if (!newValue->column_averages)
    {
        perror("Failed to allocate memory for column_averages");
        free(newValue->file_number);
        free(newValue);
        return;
    }
    newValue->next = NULL;

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        free(newValue->column_averages);
        free(newValue->file_number);
        free(newValue);
        return;
    }

    double *column_sums = calloc(max_cols, sizeof(double));
    int *column_counts = calloc(max_cols, sizeof(int));
    if (!column_sums || !column_counts)
    {
        perror("Failed to allocate memory for temporary arrays");
        free(column_sums);
        free(column_counts);
        fclose(file);
        free(newValue->column_averages);
        free(newValue->file_number);
        free(newValue);
        return;
    }

    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), file))
    {
        newValue->num_rows++;
        int column_index = 0;

        char *token = strtok(line, ",");
        while (token)
        {
            while (*token == '\n' || *token == '\r')
                token++;

            char *endptr;
            double value = strtod(token, &endptr);
            if (endptr != token && column_index < max_cols)
            {
                column_sums[column_index] += value;
                column_counts[column_index]++;
            }

            token = strtok(NULL, ",");
            column_index++;
        }
    }

    fclose(file);

    for (int i = 0; i < max_cols; i++)
    {
        if (column_counts[i] > 0)
        {
            newValue->column_averages[i] = column_sums[i] / column_counts[i];
        }
    }

    printf("\nCalculator ID: %d\n", newValue->calculator_id);
    printf("Processed File: %s\n", newValue->file_number);
    printf("Number of Rows: %d\n", newValue->num_rows);
    printf("Column Averages:\n");
    // for (int i = 0; i < max_cols; i++)
    // {
    //     if (column_counts[i] > 0)
    //     {
    //         printf("  Column %d: %.2f\n", i + 1, newValue->column_averages[i]);
    //     }
    // }

    free(column_sums);
    free(column_counts);
    free(newValue->file_number);
    free(newValue->column_averages);
    free(newValue);
}

#define MAX_FILES 100
#define FILENAME_MAX_LENGTH 256

void *calculator_thread(void *arg)
{
    CalculatorParams *params = (CalculatorParams *)arg;

    int fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0)
    {
        perror("Error opening FIFO for reading");
        return NULL;
    }

    int fifo_fd_move = open(FIFO_PATH_MOVE, O_WRONLY); // Open the FIFO for writing
    if (fifo_fd_move < 0)
    {
        perror("Error opening FIFO for writing");
        return NULL;
    }

    char file_names[MAX_FILES][FILENAME_MAX_LENGTH]; // Array to store file names
    int file_count = 0;                              // Number of files stored in the array

    while (1)
    {
        pthread_mutex_lock(&shared_memory->file_mutex);

        if (shared_memory->file_count > shared_memory->num_calculators)
        {

            char buffer[1024]; // Temporary buffer to read from FIFO
            ssize_t bytesRead = read(fifo_fd, buffer, sizeof(buffer) - 1);

            pthread_mutex_unlock(&shared_memory->file_mutex);

            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0'; // Null-terminate the string
                char *filename = strtok(buffer, "\n");

                while (filename != NULL && file_count < MAX_FILES)
                {
                    // Save the filename to the array
                    strncpy(file_names[file_count], filename, FILENAME_MAX_LENGTH - 1);
                    file_names[file_count][FILENAME_MAX_LENGTH - 1] = '\0'; // Ensure null-termination
                    file_count++;

                    filename = strtok(NULL, "\n"); // Get the next filename
                }
                shared_memory->num_calculators += file_count;

                pthread_mutex_unlock(&shared_memory->file_mutex);
            }
            else if (bytesRead == -1 && errno != EAGAIN)
            {
                perror("Error reading from FIFO");
            }

            // Process saved file names
            for (int i = 0; i < file_count; i++)
            {
                printf("Calculator %d is processing file: %s\n", params->calculator_id, file_names[i]);
                calculate_csv_file(file_names[i], params->calculator_id);
                //pthread_mutex_lock(&fifo_mutex);
                printf(" the number of file =%d\n", shared_memory->num_calculators);

                if (write(fifo_fd_move, file_names[i], strlen(file_names[i])) < 0)
                {
                    perror("Error writing to FIFO");
                }
                write(fifo_fd_move, "\n", 1);
            }

            // Clear the array after processing
            file_count = 0;
        }
        else
        {
            pthread_mutex_unlock(&shared_memory->file_mutex);
            usleep(1000); // Sleep for 1ms to avoid high CPU usage
        }
    }

    close(fifo_fd);
    return NULL;
}

// void *calculator_thread(void *arg)
// {
//     CalculatorParams *params = (CalculatorParams *)arg;

//     int fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
//     if (fifo_fd < 0)
//     {
//         perror("Error opening FIFO for reading");
//         return NULL;
//     }
//     int fifo_fd_move = open(FIFO_PATH_MOVE, O_WRONLY);
//     if (fifo_fd_move < 0)
//     {
//         perror("Error opening FIFO for writing");
//         close(fifo_fd);
//         return NULL;
//     }

//     while (1)
//     {
//         pthread_mutex_lock(&shared_memory->file_mutex);
//         // printf("Calculator %d is processing file: \n", params->calculator_id);

//         if (shared_memory->file_count > shared_memory->num_calculators)
//         {

//             shared_memory->num_calculators++;
//             char buffer[256];
//             ssize_t bytesRead = read(fifo_fd, buffer, sizeof(buffer) - 1);

//             pthread_mutex_unlock(&shared_memory->file_mutex);

//             if (bytesRead > 0)
//             {
//                 buffer[bytesRead] = '\0'; // Null-terminate the string
//                 printf("Calculator %d read from FIFO: %s\n", params->calculator_id, buffer);

//                 // Tokenize and process each filename
//                 char *filename = strtok(buffer, "\n"); // Split by newline
//                 while (filename != NULL)
//                 {
//                     // Create a local copy of the filename
//                     char temp_filename[256];
//                     strncpy(temp_filename, filename, sizeof(temp_filename) - 1);
//                     temp_filename[sizeof(temp_filename) - 1] = '\0'; // Ensure null-termination

//                     printf("Calculator is processing file: %s    is is  %s \n", temp_filename,filename);

//                     // Process the file using the temporary copy
//                     calculate_csv_file(temp_filename, params->calculator_id);

//                     // Get the next filename
//                     filename = strtok(NULL, "\n");
//                 }
//             }

//             else if (bytesRead == -1 && errno != EAGAIN)
//             {
//                 perror("Error reading from FIFO");
//             }
//         }
//         else
//         {
//             pthread_mutex_unlock(&shared_memory->file_mutex);
//             usleep(1000); // Sleep for 1ms to avoid high CPU usage
//         }
//     }

//     close(fifo_fd);
//     close(fifo_fd_move);
//     return NULL;
// }

// void calculate_csv_file(const char *filename, int calculator_id) {
//     struct MEMORYcalculator *newValue = malloc(sizeof(struct MEMORYcalculator));
//     if (!newValue) {
//         perror("Failed to allocate memory for MEMORYcalculator");
//         return;
//     }

//     newValue->calculator_id = calculator_id;
//     if (!newValue->file_number) {
//         perror("Failed to allocate memory for file_number");
//         free(newValue);
//         return;
//     }
//     strcpy(newValue->file_number, filename);

//     newValue->num_rows = 0;
//     newValue->column_averages = calloc(max_cols, sizeof(double));
//     if (!newValue->column_averages) {
//         perror("Failed to allocate memory for column_averages");
//         free(newValue->file_number);
//         free(newValue);
//         return;
//     }

//     FILE *file = fopen(filename, "r");
//     if (!file) {
//         perror("Error opening file");
//         free(newValue->column_averages);
//         free(newValue->file_number);
//         free(newValue);
//         return;
//     }

//     char line[LINE_BUFFER_SIZE];
//     double *column_sums = calloc(max_cols, sizeof(double));
//     int *column_counts = calloc(max_cols, sizeof(int));
//     if (!column_sums || !column_counts) {
//         perror("Failed to allocate memory for temporary arrays");
//         free(column_sums);
//         free(column_counts);
//         fclose(file);
//         free(newValue->column_averages);
//         free(newValue->file_number);
//         free(newValue);
//         return;
//     }

//     while (fgets(line, sizeof(line), file)) {
//         newValue->num_rows++;
//         int column_index = 0;
//         char *token = strtok(line, ",");
//         while (token) {
//             char *endptr;
//             while (*token == '\n' || *token == '\r') token++;
//             double value = strtod(token, &endptr);
//             if (endptr != token && column_index < max_cols) {
//                 column_sums[column_index] += value;
//                 column_counts[column_index]++;
//             }
//             token = strtok(NULL, ",");
//             column_index++;
//         }
//     }
//     fclose(file);

//     for (int i = 0; i < max_cols; i++) {
//         if (column_counts[i] > 0) {
//             newValue->column_averages[i] = column_sums[i] / column_counts[i];
//         }
//     }

//     printf("Calculator ID: %d\n", newValue->calculator_id);
//     printf("Processed File: %s\n", newValue->file_number);
//     printf("Number of Rows: %d\n", newValue->num_rows);
//     printf("Column Averages:\n");
//     for (int i = 0; i < max_cols && column_counts[i] > 0; i++) {
//         printf("  Column %d: %.2f\n", i + 1, newValue->column_averages[i]);
//     }

//     free(column_sums);
//     free(column_counts);
//     free(newValue);
//     newValue = NULL;
// }