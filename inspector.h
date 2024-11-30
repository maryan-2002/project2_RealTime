#ifndef INSPECTOR_H
#define INSPECTOR_H
// Define constants
#define HOME_DIR "home"
#define UNPROCESSED_DIR "home/UnProcessed"
#define MAX_FILENAME_LENGTH 256

// Shared data structure
struct SharedData {
    int unprocessed_count;
};

// Function declarations
void create_directory_if_not_exists(const char *dir_name);
int is_csv_file(const char *filename);
int is_file_older_than(const char *filepath, int age_in_seconds);
void inspect_and_move_csv_files(int age_in_seconds, struct SharedData *shared_data);
void *inspector_thread(void *arg);

#endif // INSPECTOR_H
