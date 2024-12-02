#include "inspector.h"
#include "header.h"
#include <dirent.h>
#include "globel.c"
#define _DEFAULT_SOURCE
#include <dirent.h>
#define BACKUP_DIR "home/Backup"

void create_directory_if_not_exists(const char *dir_name)
{
    struct stat st = {0};
    if (stat(dir_name, &st) == -1)
    {
        if (mkdir(dir_name, 0755) == -1)
        {
            perror("Failed to create directory");
        }
    }
}

int is_csv_file(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    return ext && strcmp(ext, ".csv") == 0;
}


int is_file_older_than(const char *filepath, int age_in_seconds)
{
    struct stat file_stat;
    if (stat(filepath, &file_stat) == -1)
    {
        perror("Failed to stat file");
        return 0;
    }
    time_t current_time = time(NULL);
    return (current_time - file_stat.st_mtime) > age_in_seconds;
}

// General function to save a file name to any specified text file
void save_to_text_file(const char *filename, const char *target_file) {
    // Extract just the filename (e.g., "4.csv" from "home/4.csv")
    const char *basename = strrchr(filename, '/');
    if (basename != NULL) {
        basename++;  // Skip the '/' character to get just the filename
    } else {
        basename = filename;  // If no '/' is found, use the original filename
    }

    // Open the target file for appending
    FILE *file = fopen(target_file, "a");
    if (file == NULL) {
        perror("Error opening the target file for writing");
        return;
    }

    // Write the extracted filename to the target file
    fprintf(file, "%s\n", basename);

    // Close the file
    fclose(file);
}

void inspect_and_move_csv_files(int age_in_seconds, const char *source_dir, const char *dest_dir)
{
    DIR *dir = opendir(source_dir);
    if (!dir)
    {
        perror("Failed to open source directory");
        return;
    }

    create_directory_if_not_exists(dest_dir);

    struct dirent *entry;
    char filepath[MAX_FILENAME_LENGTH];
    char destpath[MAX_FILENAME_LENGTH];
    struct stat file_stat;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", source_dir, entry->d_name);
        if (stat(filepath, &file_stat) == 0 && S_ISREG(file_stat.st_mode))
        {
            if (is_csv_file(entry->d_name))
            {
                if (is_file_older_than(filepath, age_in_seconds))
                {
                    snprintf(destpath, sizeof(destpath), "%s/%s", dest_dir, entry->d_name);
                    if (rename(filepath, destpath) == -1)
                    {
                        perror("Failed to move file");
                    }
                    else
                    {
                        pthread_mutex_lock(&shared_mutex_inspector);
                        shared_memory->unprocessed_count++;
                        //printf(" the number of unprocessed_count file is : %d \n", shared_memory->unprocessed_count);
                        save_to_text_file(filepath, "UnProcessed.txt");

                        if (shared_memory->unprocessed_count == unprocees_th)
                        {
                            printf("End of the grogram\n");

                            kill_all_and_exit();
                        }
                        pthread_mutex_unlock(&shared_mutex_inspector);
                    }
                }
            }
        }
    }
    closedir(dir);
}


void inspect_and_move_csv_files2(int age_in_seconds, const char *source_dir, const char *dest_dir)
{
    DIR *dir = opendir(source_dir);
    if (!dir)
    {
        perror("Failed to open source directory");
        return;
    }

    create_directory_if_not_exists(dest_dir);

    struct dirent *entry;
    char filepath[MAX_FILENAME_LENGTH];
    char destpath[MAX_FILENAME_LENGTH];
    struct stat file_stat;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", source_dir, entry->d_name);
        if (stat(filepath, &file_stat) == 0 && S_ISREG(file_stat.st_mode))
        {
            if (is_csv_file(entry->d_name))
            {
                if (is_file_older_than(filepath, age_in_seconds))
                {
                    snprintf(destpath, sizeof(destpath), "%s/%s", dest_dir, entry->d_name);
                    if (rename(filepath, destpath) == -1)
                    {
                        // perror("Failed to move file");
                    }
                    else
                    {
                        pthread_mutex_lock(&shared_mutex_backup);
                        shared_memory->backup_count++;
                        save_to_text_file(filepath, "Backup.txt");

                        //printf(" the number of backup file is : %d \n", shared_memory->backup_count);

                        if (shared_memory->backup_count == backup_th)
                        {
                            printf("End of the grogram\n");

                            kill_all_and_exit();
                        }
                        pthread_mutex_unlock(&shared_mutex_backup);
                    }
                }
            }
        }
    }

    closedir(dir);
}



void inspect_and_delete_csv_files(int age_in_seconds, const char *source_dir)
{
    DIR *dir = opendir(source_dir);
    if (!dir)
    {
        perror("Failed to open source directory");
        return;
    }

    struct dirent *entry;
    char filepath[MAX_FILENAME_LENGTH];
    struct stat file_stat;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", source_dir, entry->d_name);

        if (stat(filepath, &file_stat) == 0 && S_ISREG(file_stat.st_mode))
        {
            if (is_csv_file(entry->d_name))
            {
                if (is_file_older_than(filepath, age_in_seconds))
                {
                    if (remove(filepath) == -1)
                    {
                        // perror("Failed to delete file");
                    }
                    else
                    {

                        pthread_mutex_lock(&shared_mutex_deleate);
                        shared_memory->deleted_count++;
                        save_to_text_file(filepath, "Delete.txt");

                        //printf(" the number of delated file is : %d \n", shared_memory->deleted_count);
                        if (shared_memory->deleted_count == delete_th)
                        {
                            printf("End of the grogram\n");
                            kill_all_and_exit();
                        }
                        pthread_mutex_unlock(&shared_mutex_deleate);
                    }
                }
            }
        }
    }

    closedir(dir);
}

void *inspector_thread_type_1(void *arg)
{
    while (1)
    {
        inspect_and_move_csv_files(unprocessed_value, HOME_DIR, UNPROCESSED_DIR);
        sleep(5);
    }
}

void *inspector_thread_type_2(void *arg)
{
    while (1)
    {
        inspect_and_move_csv_files2(MAX_BACKUP, PROCESSED_DIR, BACKUP_DIR);
        sleep(5);
    }
}

void *inspector_thread_type_3(void *arg)
{
    while (1)
    {
        inspect_and_delete_csv_files(delete_value, BACKUP_DIR);
        sleep(5);
    }
}
