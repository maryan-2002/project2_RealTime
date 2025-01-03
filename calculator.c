#include "header.h"
#include "calculator.h"
#include "globel.c"

#define MAX_FILES 100
#define FILENAME_MAX_LENGTH 256



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

            // Lock the shared structure
            pthread_mutex_lock(&calc.mutex);

            int min_updated = 0, max_updated = 0;

            // Check and update min
            if (newValue->column_averages[i] < calc.min_average)
            {
                calc.min_average = newValue->column_averages[i];
                calc.min_column = i;
                strncpy(calc.min_file, filename, sizeof(calc.min_file));
                min_updated = 1;
            }

            // Check and update max
            if (newValue->column_averages[i] > calc.max_average)
            {
                calc.max_average = newValue->column_averages[i];
                calc.max_column = i;
                strncpy(calc.max_file, filename, sizeof(calc.max_file));
                max_updated = 1;
            }

            // Print updates only if there is a change
            if (min_updated)
            {
                printf("[UPDATED MIN] New Min: %.2f (Column: %d, File: %s)\n",
                       calc.min_average, calc.min_column, calc.min_file);
            }

            if (max_updated)
            {
                printf("[UPDATED MAX] New Max: %.2f (Column: %d, File: %s)\n",
                       calc.max_average, calc.max_column, calc.max_file);
            }

            pthread_mutex_unlock(&calc.mutex);
        }
    }

     printf("\nCalculator ID: %d\n", newValue->calculator_id);
    printf("Processed File: %s\n", newValue->file_number);
    newValue->num_rows--;
    printf("Number of Rows: %d\n", newValue->num_rows);

    printf("Column Averages: [");
    for (int i = 0; i < max_cols; i++)
    {
        if (column_counts[i] > 0)
        {
            printf(" %.2f ", newValue->column_averages[i]);
        }
    }
    

    free(column_sums);
    free(column_counts);
    // Extract the file name from the full path
    const char *just_filename = strrchr(filename, '/');
    if (!just_filename)
        just_filename = filename; // No '/' found, use the full string
    else
        just_filename++; // Move past the '/'

    // Write the file name to Processed.txt
    FILE *processed_file = fopen("Processed.txt", "a");
    if (!processed_file)
    {
        perror("Error opening Processed.txt");
    }
    else
    {
        fprintf(processed_file, "%s\n", just_filename);
        fclose(processed_file);
    }
    // Add newValue to the linked list
    pthread_mutex_lock(&calc.mutex);
    newValue->next = calc.calculators;
    calc.calculators = newValue;
    pthread_mutex_unlock(&calc.mutex);
}

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
        if (shared_memory->num_calculators == procees_th)
        {
            printf("   ennnnnnnnnnnnnnnd from calaulated\n");
            kill_all_and_exit();
        }

        if (shared_memory->file_count > shared_memory->num_calculators)
        {
            shared_memory->num_calculators++;

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
            }
            else if (bytesRead == -1 && errno != EAGAIN)
            {
                perror("Error reading from FIFO");
            }

            printf("Calculator %d is processing file: %s\n", params->calculator_id, file_names[0]);
            calculate_csv_file(file_names[0], params->calculator_id);
            if (write(fifo_fd_move, file_names[0], strlen(file_names[0])) < 0)
            {
                perror("Error writing to FIFO");
            }
            write(fifo_fd_move, "\n", 1);

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
