#ifndef INSPECTOR_H
#define INSPECTOR_H

#define HOME_DIR "home"
#define UNPROCESSED_DIR "home/UnProcessed"
#define PROCESSED_DIR "home/Processed"
#define MAX_FILENAME_LENGTH 256
#define MAX_TIME_VALUE 86400

struct SharedData {
    int unprocessed_count;
};

void create_directory_if_not_exists(const char *dir_name);
int is_csv_file(const char *filename);
int is_file_older_than(const char *filepath, int age_in_seconds);
void inspect_and_move_csv_files(int age_in_seconds, const char *source_dir, const char *dest_dir);
void inspect_and_delete_csv_files(int age_in_seconds, const char *source_dir);
void *inspector_thread_type_1(void *arg);
void *inspector_thread_type_2(void *arg);
void *inspector_thread_type_3(void *arg);
void *inspector_thread(void *arg);

#endif // INSPECTOR_H
