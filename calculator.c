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
    newValue->next = NULL; // New node is initially not pointing to any other node

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
            char *endptr;
            while (*token == '\n' || *token == '\r')
                token++;
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

    pthread_mutex_lock(&calc.mutex);

    if (calc.calculators == NULL)
    {
        calc.calculators = newValue; // If the list is empty, the new node becomes the head
    }
    else
    {
        struct MEMORYcalculator *current = calc.calculators;
        while (current->next != NULL)
        { // Traverse to the last node
            current = current->next;
        }
        current->next = newValue; // Append the new node to the end of the list
    }

    pthread_mutex_unlock(&calc.mutex);
    printf("Calculator ID: %d\n", newValue->calculator_id);
    printf("Processed File: %s\n", newValue->file_number);
    printf("Number of Rows: %d\n", newValue->num_rows);
    printf("Column Averages:\n");
    // for (int i = 0; i < max_cols && column_counts[i] > 0; i++)
    // {
    //     printf("  Column %d: %.2f\n", i + 1, newValue->column_averages[i]);
    // }
    printf(" doooneee baby");
    free(column_sums);
    free(column_counts);
}

void *calculator_thread(void *arg)
{
    CalculatorParams *params = (CalculatorParams *)arg;

    int fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0) {
        perror("Error opening FIFO for reading");
        return NULL;
    }
    int fifo_fd_move =open(FIFO_PATH_MOVE, O_WRONLY);
    if (fifo_fd_move < 0) {
        perror("Error opening FIFO for writing");
        close(fifo_fd);
        return NULL;
    }

    while (1) {
        pthread_mutex_lock(&shared_memory->file_mutex);

        if (shared_memory->file_count > shared_memory->num_calculators) {
            shared_memory->num_calculators++;
            pthread_mutex_unlock(&shared_memory->file_mutex);

            char buffer[256];
            ssize_t bytesRead = read(fifo_fd, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0'; // Null-terminate the string
                char *filename = strtok(buffer, "\n");
                calculate_csv_file(filename, params->calculator_id);
                write(fifo_fd_move, filename, strlen(filename) + 1); // Write the filename to FIFO
            } else if (bytesRead == -1 && errno != EAGAIN) {
                perror("Error reading from FIFO");
            }
        } else {
            pthread_mutex_unlock(&shared_memory->file_mutex);
            usleep(1000); // Sleep for 1ms to avoid high CPU usage
        }
    }

    close(fifo_fd);
    close(fifo_fd_move);
    return NULL;
}

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