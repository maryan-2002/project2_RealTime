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

int num_type1 = 1;
int num_type2 = 1;
int num_type3 = 1;
int age_limit = 86400;

int procees_th = 50;
int unprocees_th = 100;
int backup_th = 20;
int delete_th = 100;
int runtime_th = 60;

pthread_mutex_t fifo_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shared_mutex_inspector = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shared_mutex_backup = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shared_mutex_deleate = PTHREAD_MUTEX_INITIALIZER;
pthread_t generator_threads[30];
pthread_t type1_threads[30];
pthread_t type2_threads[30];
pthread_t type3_threads[30];
pthread_t mover_threads[30];
pthread_t calculator_threads[30];
pthread_t opengl_thread;
void initGraphics(int argc, char *argv[]); // Add the function prototype 


struct sembuf acquire = {0, -1, SEM_UNDO}, release = {0, 1, SEM_UNDO};
// Shared semaphore ID
int sem_id;
struct MEMORY *shared_memory; // Shared memory pointer
struct SharedCalculators calc;


// Function to delete specific .txt files in the main directory
void delete_txt_files_in_main_dir() {
    // List of files to delete
    const char *files_to_delete[] = {
        "home.txt",
        "Backup.txt",
        "Processed.txt",
        "UnProcessed.txt",
        "delete.txt",  // Ensure 'Delete.txt' is listed here
        "data.txt",
        "Processed1.txt",
        "UnProcessed1.txt",
        "Backup1.txt",
        "delete1.txt"
    };

    // Get the current working directory (main directory)
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        for (int i = 0; i < 10; i++) {  // Update to delete 6 files
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", cwd, files_to_delete[i]);

            // Debug: Print full file path
            // printf("Trying to delete: %s\n", file_path);

            // Check if the file exists before attempting to delete
            if (access(file_path, F_OK) == 0) {
                if (unlink(file_path) == 0) {
                    // printf("Deleted: %s\n", file_path);
                } else {
                    perror("Error deleting file");
                }
            } else {
                printf("File %s not found.\n", file_path);  // Debug: File not found
            }
        }
    } else {
        perror("getcwd() error");
    }
}

// Function to create specific .txt files in the main directory
void create_txt_files_in_main_dir() {
    const char *files_to_create[] = {
        "home.txt",
        "Backup.txt",
        "Processed.txt",
        "UnProcessed.txt",
        "delete.txt",   // Ensure 'Delete.txt' is correctly added here
        "Processed1.txt",
        "UnProcessed1.txt",
        "Backup1.txt",
        "delete1.txt"
    };

    // Get the current working directory (main directory)
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        for (int i = 0; i < 10; i++) {  // Update the iteration count to 6
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", cwd, files_to_create[i]);

            // Create the file by opening it in write mode
            FILE *file = fopen(file_path, "w");
            if (file) {
                // printf("Created: %s\n", file_path);
                fclose(file);  // Close the file
            } else {
                perror("Error creating file");
            }
        }
    } else {
        perror("getcwd() error");
    }
}

// Function to delete .csv files in a directory
void delete_csv_files(const char *directory_path) {
    DIR *dir;
    struct dirent *entry;

    // printf("Trying to open directory: %s\n", directory_path);

    if (access(directory_path, F_OK) != 0) {
        perror("Directory does not exist or cannot be accessed");
        return;
    }

    dir = opendir(directory_path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Ensure it's a regular file
            if (strcasecmp(strrchr(entry->d_name, '.'), ".csv") == 0) {
                char file_path[1024];
                snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

                if (unlink(file_path) == 0) {
                    // printf("Deleted: %s\n", file_path);
                } else {
                    perror("Error deleting file");
                }
            }
        }
    }

    closedir(dir);
}

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

// Terminate the OpenGL thread gracefully
void terminate_opengl_thread(pthread_t *thread) {
    // Request thread cancellation
    if (pthread_cancel(*thread) != 0) {
        perror("Failed to cancel OpenGL thread");
    }

    // Wait for the thread to terminate
    if (pthread_join(*thread, NULL) != 0) {
        perror("Failed to join OpenGL thread");
    } else {
        printf("OpenGL thread terminated successfully.\n");
    }
}


void cleanupGraphics() {
    // Close and clean up the OpenGL context and window
    glutDestroyWindow(glutGetWindow());
    printf("Graphics and window resources cleaned up.\n");
}

// Function to kill all threads and exit
void kill_all_and_exit()
{
    terminate_opengl_thread(&opengl_thread);
    // fflush(stdout);
    // Cancel generator threads
    for (int i = 0; i < num_generators; i++)
    {
        pthread_cancel(generator_threads[i]);
    }

    // Cancel calculator threads
    for (int i = 0; i < num_calculators; i++)
    {
        pthread_cancel(calculator_threads[i]);
    }

    // Cancel mover threads
    for (int i = 0; i < num_movers; i++)
    {
        pthread_cancel(mover_threads[i]);
    }
    for (int i = 0; i < num_type1; i++)
    {
        pthread_cancel(type1_threads[i]);
    }
    // for (int i = 0; i < num_type2; i++)
    // {
    //     pthread_cancel(type2_threads[i]);
    // }
    // for (int i = 0; i < num_type3; i++)
    // {
    //     pthread_cancel(type3_threads[i]);
    // }

    if (unlink(FIFO_PATH) < 0)
    {
        perror("Error deleting FIFO");
    }
    else
    {
        printf("FIFO %s deleted successfully.\n", FIFO_PATH);
    }

    if (unlink(FIFO_PATH_MOVE) < 0)
    {
        perror("Error deleting FIFO for movers");
    }
    else
    {
        printf("FIFO %s deleted successfully.\n", FIFO_PATH_MOVE);
    }

    cleanupGraphics(); // Clean up OpenGL context and resources

    printf("\n\n END PROGRAM \n");
    printf("All threads terminated forcefully. Exiting program.\n");
    // system("rm -r " UNPROCESSED_DIR);
    // system("rm -r " PROCESSED_DIR);
    // exit(EXIT_SUCCESS);
}

// Function for OpenGL rendering thread
void *startOpenGL(void *arg) {
    initGraphics(0, NULL); // Initialize and start OpenGL rendering
    return NULL;
}

int main(int argc, char *argv[]) {
    


    // Start the OpenGL thread
    if (pthread_create(&opengl_thread, NULL, startOpenGL, NULL) != 0) {
        perror("Failed to create OpenGL thread");
        return EXIT_FAILURE;
    }
    // Step 1: Delete and create necessary text files
    delete_txt_files_in_main_dir();
    create_txt_files_in_main_dir();

    // Step 2: Clean up and set up the working directories
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Create paths by appending subdirectories to the current directory
        char home_dir[1024], backup_dir[1024], processed_dir[1024], unprocessed_dir[1024];
        snprintf(home_dir, sizeof(home_dir), "%s/home", cwd);
        snprintf(backup_dir, sizeof(backup_dir), "%s/home/Backup", cwd);
        snprintf(processed_dir, sizeof(processed_dir), "%s/home/Processed", cwd);
        snprintf(unprocessed_dir, sizeof(unprocessed_dir), "%s/home/UnProcessed", cwd);
        
        // Call the delete function for each directory
        delete_csv_files(home_dir);
        delete_csv_files(backup_dir);
        delete_csv_files(processed_dir);
        delete_csv_files(unprocessed_dir);
    } else {
        perror("getcwd() error");
    }

    // Step 3: Read arguments from the configuration file
    if (read_arguments_from_file("arguments.txt") != 0) {
        return EXIT_FAILURE;
    }

    // Step 4: Starting generators
    printf("Starting %d file generators with time range [%d, %d] seconds.\n", num_generators, min_time, max_time);
    printf("Global settings: %d rows, %d cols, value range [%.2d, %.d], miss percentage: %d%%\n", max_rows, max_cols, min_value, max_value, miss_percentage);

    // Seed random number generator
    srand(time(NULL));

    if (mkfifo(FIFO_PATH, 0666) < 0) {
        // perror("Error creating FIFO");
    }
    if (mkfifo(FIFO_PATH_MOVE, 0666) < 0) {
        // perror("Error creating FIFO for movers");
    }

    init_semaphore();
    initialize_fifo_mutex();
    
    // Step 5: Initialize file generators and threads for moving files
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
    CalculatorParams calculator_params[num_calculators];
    for (int i = 0; i < num_calculators; i++) {
        calculator_params[i].calculator_id = i + 1;
        calculator_params[i].max_cols = max_cols;

        if (pthread_create(&calculator_threads[i], NULL, calculator_thread, &calculator_params[i]) != 0) {
            perror("Failed to create calculator thread");
            return EXIT_FAILURE;
        }
    }

    // Step 6: Create threads for file movers
    for (int i = 0; i < num_movers; i++) {
        if (pthread_create(&mover_threads[i], NULL, mover_thread, NULL) != 0) {
            perror("Failed to create mover thread");
            return EXIT_FAILURE;
        }
    }

    // Step 7: Create inspector threads
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

    // Wait for a specified runtime duration before finishing
    sleep(runtime_th * 60);
    printf("Time is up.\n");

    // Step 9: Join threads and clean up
    for (int i = 0; i < num_generators; i++) {
        pthread_join(generator_threads[i], NULL);
    }

    for (int i = 0; i < num_calculators; i++) {
        pthread_join(calculator_threads[i], NULL);
    }

    for (int i = 0; i < num_movers; i++) {
        pthread_join(mover_threads[i], NULL);
    }

    for (int i = 0; i < num_type1; i++) {
        pthread_join(type1_threads[i], NULL);
    }

    for (int i = 0; i < num_type2; i++) {
        pthread_join(type2_threads[i], NULL);
    }

    for (int i = 0; i < num_type3; i++) {
        pthread_join(type3_threads[i], NULL);
    }
        pthread_join(opengl_thread, NULL);


    return EXIT_SUCCESS;
}