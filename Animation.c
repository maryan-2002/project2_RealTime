#ifdef _APPLE_CC_
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "header.h"



#define MAX_FILENAME_LEN 50
#define CHECK_INTERVAL 1000 // Time interval in milliseconds for checking the file
#define ROW_HEIGHT 0.12f
#define MAX_FILENAME_LEN 50
#define MAX_LINE_LEN 256
// Number of rows to display at a time
#define ROWS_PER_PAGE 6
#define MAX_ROWS 100
// Adjust the scrollbar positions
float scrollbarX = 0.98f; // X position of the scrollbar (closer to the table)
float scrollbarYStart = 0.53f; // Start Y position of the scrollbar (below the first row)
float scrollbarYEnd = -0.187f; // Bottom Y position of the scrollbar
float scrollbarWidth = 0.03f; // Width of the scrollbar
float scrollbarThumbY = 0.53f; // Initial Y position of the scrollbar thumb
float scrollbarThumbHeight; // Dynamically calculated height of the scrollbar thumb

bool draggingScrollbar = false; // Flag to check if the scrollbar is being dragged
float dragStartY = 0.0f; // Starting Y position of the drag

typedef struct {
    int calculator_id;
    char file_number[50];
    int num_rows;
    float column_averages[50];
} CalculatorData;

typedef struct {
    char fileName[MAX_FILENAME_LEN];
} FileList;


CalculatorData displayedData;




typedef struct {
    int generatorID;
    char fileName[MAX_FILENAME_LEN];
    int rows;
    int columns;
} RowData;
// Global variables
RowData *rowData = NULL;
int totalDataRows = 0;
int allocatedRows = 0;
int startRow = 0;           // Starting row for the visible table
int maxVisibleRows = 6;     // Number of rows visible at a time
const float rowHeight = 0.12f;  // Height of a row
char filename[MAX_FILENAME_LEN] = "data.txt";
time_t lastModifiedTime = 0;
float windowHeight = 800.0f; // Default window height
// File position variables
float fileX = -0.2f;
float fileY = 0.6f;
char movingFile[MAX_FILENAME_LEN] = "";
int stage = 0; // Animation stage: 0 = Home -> Processed, etc.

// Timer variable
float timer = 0.0f;
// File counters for each folder
int homeFiles = 0;  // Default initialization

int processedFiles = 0;
int unprocessedFiles = 0;
int backupFiles = 0;
int recycleBinFiles = 0;

// Global variable to control the scroll offset
float scrollOffset = 0.0f;
const int totalRows = 6;        // Total number of rows in your table
const float tableHeight = rowHeight * totalRows; // The height of all rows combined
// Adjust this to set the number of visible rows
const int VISIBLE_ROWS = 6;

FileList *homeFileList = NULL;
int homeFileCount = 0;
FileList *unProcessedFileList = NULL;  // Declare for UnProcessed.txt files
int unProcessedFileCount = 0;  // Count for UnProcessed.txt files

FileList *processedFileList = NULL;
int processedFileCount = 0;

FileList *backupList = NULL;
int backupFileCount = 0;


bool isPaperVisible = false;      // To control paper visibility
char currentFile[MAX_FILENAME_LEN] = ""; // File name currently being moved
float paperX = -0.2f;             // Starting X position for paper
float paperY = 0.6f;              // Starting Y position for paper

void processFilesWrapper(int value);
void finalizeFileMove(int value);
void checkAndProcessBackup(int value);




void processFilesWrapper(int value) {
    processFiles(); // Call the actual processFiles function
}


// Draw text on the screen
void drawText(float x, float y, const char *text, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

int countFilesInProcessed1(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        printf("Error: Unable to open %s\n", fileName);
        return 0;
    }

    int lineCount = 0;
    char line[MAX_FILENAME_LEN];

    // Count lines in the file
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '\0' && line[0] != '\n') {
            lineCount++;
        }
    }

    fclose(file);
    return lineCount;
}


void drawPaperFile(float xOffset, float yOffset) {
    if (!isPaperVisible) return; // Only draw if the paper is visible

    glColor3f(1.0f, 1.0f, 1.0f); // White color for the paper
    glBegin(GL_POLYGON);
    glVertex2f(-0.05f + paperX, -0.1f + paperY); // Use paperX and paperY
    glVertex2f(0.05f + paperX, -0.1f + paperY);
    glVertex2f(0.05f + paperX, 0.1f + paperY);
    glVertex2f(-0.05f + paperX, 0.1f + paperY);
    glEnd();

    // Draw lines inside the file (paper lines)
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for lines
    float lineSpacing = 0.015f;
    for (float i = 0.05f; i > -0.05f; i -= lineSpacing) {
        glBegin(GL_LINES);
        glVertex2f(-0.04f + paperX, i + paperY);
        glVertex2f(0.04f + paperX, i + paperY);
        glEnd();
    }

    // Draw the filename inside the paper
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for text
    drawText(paperX - 0.02f, paperY, currentFile, 0.0f, 0.0f, 0.0f); // Draw current file name
}


// Function to draw a background
void drawBackground() {
    glColor3f(0.2078f, 0.3608f, 0.4902f); // Background color #355c7d
    glBegin(GL_POLYGON);
    glVertex2f(-1.0f, 1.0f);   // Top-left corner
    glVertex2f(1.0f, 1.0f);    // Top-right corner
    glVertex2f(1.0f, -1.0f);   // Bottom-right corner
    glVertex2f(-1.0f, -1.0f);  // Bottom-left corner
    glEnd();
}
// Function to draw folders with original shapes
void drawFolder(float xOffset, float yOffset) {
    // Draw the main body of the folder
    glColor3f(1.0f, 0.9f, 0.0f);  // Yellow color
    glBegin(GL_POLYGON);
    glVertex2f(-0.15f + xOffset, -0.1f + yOffset); // Left-bottom corner
    glVertex2f(0.15f + xOffset, -0.1f + yOffset);  // Right-bottom corner
    glVertex2f(0.15f + xOffset, 0.05f + yOffset);  // Right-top corner
    glVertex2f(-0.15f + xOffset, 0.05f + yOffset); // Left-top corner
    glEnd();

    // Draw the shadow for the main body
    glColor3f(0.8f, 0.7f, 0.0f);  // Darker yellow color for shadow
    glBegin(GL_POLYGON);
    glVertex2f(-0.15f + xOffset, -0.1f + yOffset); // Left-bottom corner
    glVertex2f(-0.1f + xOffset, -0.15f + yOffset); // Left shadow extension
    glVertex2f(0.1f + xOffset, -0.15f + yOffset);  // Right shadow extension
    glVertex2f(0.15f + xOffset, -0.1f + yOffset);  // Right-bottom corner
    glEnd();

    // Draw the top flap of the folder
    glColor3f(1.0f, 0.8f, 0.0f);  // Slightly lighter yellow
    glBegin(GL_POLYGON);
    glVertex2f(-0.1f + xOffset, 0.05f + yOffset);  // Left-bottom corner
    glVertex2f(0.05f + xOffset, 0.15f + yOffset);  // Top-left corner
    glVertex2f(0.15f + xOffset, 0.15f + yOffset);  // Top-right corner
    glVertex2f(0.15f + xOffset, 0.05f + yOffset);  // Right-bottom corner
    glEnd();

    // Draw the shadow for the top flap
    glColor3f(0.9f, 0.7f, 0.0f);  // Darker yellow color for shadow
    glBegin(GL_POLYGON);
    glVertex2f(-0.1f + xOffset, 0.05f + yOffset);  // Left-bottom corner
    glVertex2f(-0.05f + xOffset, 0.0f + yOffset);  // Left shadow corner
    glVertex2f(0.15f + xOffset, 0.0f + yOffset);   // Right shadow corner
    glVertex2f(0.15f + xOffset, 0.05f + yOffset);  // Right-bottom corner
    glEnd();
}
// Function to draw the recycle bin (with original shape)
void drawRecycleBin(float xOffset, float yOffset) {
    // Draw the main body of the Recycle Bin
    glColor3f(0.7f, 0.7f, 0.7f); // Grey
    glBegin(GL_POLYGON);
    glVertex2f(-0.2f + xOffset, -0.2f + yOffset);
    glVertex2f(0.2f + xOffset, -0.2f + yOffset);
    glVertex2f(0.15f + xOffset, 0.2f + yOffset);
    glVertex2f(-0.15f + xOffset, 0.2f + yOffset);
    glEnd();

    // Recycling symbol
    glColor3f(0.0f, 0.5f, 0.0f); // Green
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f + xOffset, 0.05f + yOffset);
    glVertex2f(-0.05f + xOffset, 0.0f + yOffset);
    glVertex2f(0.05f + xOffset, 0.0f + yOffset);
    glEnd();
}
int countLinesInDelete1(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        printf("Error: Unable to open %s\n", fileName);
        return 0;
    }

    int lineCount = 0;
    char line[MAX_FILENAME_LEN];

    // Count non-empty lines
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '\0' && line[0] != '\n') {
            lineCount++;
        }
    }

    fclose(file);
    return lineCount;
}

void updateAnimation(int value) {
    static bool processedIncremented = false;
    static bool backupIncremented = false;
    static bool recycleBinIncremented = false;
    static bool unprocessedIncremented = false;

    // Easing parameters
    static float speedX = 0.1f; // Horizontal speed
    static float speedY = 0.1f; // Vertical speed
    static float targetX, targetY;

    if (isPaperVisible) {
        // Determine the target positions for each stage
        if (stage == 0) { // Home to Processed
            targetX = -0.7f;
            targetY = 0.6f; // No vertical movement
        } else if (stage == 1) { // Processed to Backup
            targetX = -0.7f;
            targetY = -0.1f; // Move down
        } else if (stage == 3) { // Backup to Recycle Bin
            targetX = -0.7f;
            targetY = -0.55f; // Move further down
        } else if (stage == 4) { // Home to Unprocessed
            targetX = -0.2f; // Update as per "Unprocessed" folder's location
            targetY = -0.1f; // Update as per "Unprocessed" folder's location
        }

        // Apply easing for smooth movement
        float dx = targetX - paperX;
        float dy = targetY - paperY;

        if (fabs(dx) > 0.01f || fabs(dy) > 0.01f) { // Continue moving if not at target
            paperX += dx * speedX; // Adjust position with easing
            paperY += dy * speedY;

            glutPostRedisplay(); // Request redraw
            glutTimerFunc(30, updateAnimation, 0); // Continue animation
        } else {
            // When the paper reaches the target
            paperX = targetX;
            paperY = targetY;

            if (stage == 0 && !processedIncremented) {
                processedFiles++; // Increment Processed count
                processedIncremented = true;
            } else if (stage == 1 && !backupIncremented) {
                backupFiles++; // Increment Backup count
                backupIncremented = true;
            } else if (stage == 3 && !recycleBinIncremented) {
                recycleBinFiles++; // Increment Recycle Bin count
                recycleBinIncremented = true;
            } else if (stage == 4 && !unprocessedIncremented) {
                unprocessedFiles++; // Increment Unprocessed count
                unprocessedIncremented = true;
            }

            // Reset after reaching the destination
            updateFileCounts();
            isPaperVisible = false; // Hide the paper
            processedIncremented = false;
            backupIncremented = false;
            recycleBinIncremented = false;
            unprocessedIncremented = false;
            stage = 0; // Reset stage
        }
    }
}


int countNonEmptyLinesInBackup1(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        printf("Error: Unable to open %s\n", fileName);
        return 0;
    }

    int lineCount = 0;
    char line[MAX_FILENAME_LEN];

    // Count non-empty lines
    while (fgets(line, sizeof(line), file)) {
        // Check if the line is non-empty
        if (line[0] != '\0' && line[0] != '\n') {
            lineCount++;
        }
    }

    fclose(file);
    return lineCount;
}

void checkFilesForUpdates(int value) {
    // Read the home.txt file
    homeFileCount = readFileList("home.txt", &homeFileList);

    // Read the Processed.txt file
    processedFileCount = readFileList("Processed.txt", &processedFileList);

    // Process files if conditions are met
    processFiles();

    // Schedule the next check
    glutTimerFunc(CHECK_INTERVAL, checkFilesForUpdates, 0);
}




// Function to draw a regular circle
void drawCircle(float x, float y, float radius) {
    int numSegments = 50;  // Number of segments to approximate the circle
    glBegin(GL_POLYGON);
    for (int i = 0; i < numSegments; i++) {
        float angle = 2 * M_PI * i / numSegments;
        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
    }
    glEnd();
}
// Draw file count inside a circle
void drawFileCountInCircle(float x, float y, int fileCount) {
    // Draw the circle
    glColor3f(0.0f, 1.0f, 0.0f); // Green color
    drawCircle(x, y, 0.05f);

    // Draw the file count as text
    char countText[10];
    snprintf(countText, sizeof(countText), "%d", fileCount);
    glColor3f(1.0f, 1.0f, 1.0f); // White color for text
    drawText(x - 0.015f, y - 0.02f, countText, 1.0f, 1.0f, 1.0f);
}

// Update file counts from the text files
void updateFileCounts() {
    // Read counts from respective files
    homeFileCount = readFileList("home.txt", &homeFileList);
    processedFileCount = readFileList("Processed.txt", &processedFileList);
    backupFileCount = readFileList("Backup.txt", &backupList);

    // Count files in Processed1.txt and Backup1.txt
    processedFiles = countFilesInProcessed1("Processed1.txt");
    backupFiles = countNonEmptyLinesInBackup1("Backup1.txt");

    // Count files in delete1.txt (Recycle Bin)
    recycleBinFiles = countLinesInDelete1("delete1.txt");

    // Count files in both UnProcessed.txt and UnProcessed1.txt
    unProcessedFileCount = readFileList("UnProcessed.txt", &unProcessedFileList);
    int unProcessed1FileCount = countFilesInProcessed1("UnProcessed1.txt");
    unprocessedFiles = unProcessedFileCount + unProcessed1FileCount;

    // Redraw display with updated counts
    glutPostRedisplay();
}



void compareAndUpdateBackup() {
    FileList *processed1List = NULL;
    FileList *backupList = NULL;

    int processed1Count = readFileList("Processed1.txt", &processed1List);
    int backupCount = readFileList("Backup.txt", &backupList);

    if (processed1Count == 0 || backupCount == 0) {
        free(processed1List);
        free(backupList);
        return;
    }

    char *firstBackupFile = backupList[0].fileName;
    bool foundMatch = false;

    for (int i = 0; i < processed1Count; i++) {
        if (strcmp(firstBackupFile, processed1List[i].fileName) == 0) {
            foundMatch = true;

            // Set up animation for Processed to Backup
            strcpy(currentFile, firstBackupFile);
            isPaperVisible = true;
            paperX = -0.7f;
            paperY = 0.6f;
            stage = 1;

            // Append to Backup1.txt
            FILE *backup1File = fopen("Backup1.txt", "a");
            fprintf(backup1File, "%s\n", firstBackupFile);
            fclose(backup1File);

            // Remove from Processed1.txt
            for (int j = i; j < processed1Count - 1; j++) {
                strcpy(processed1List[j].fileName, processed1List[j + 1].fileName);
            }
            processed1Count--;

            // Remove from Backup.txt
            for (int j = 0; j < backupCount - 1; j++) {
                strcpy(backupList[j].fileName, backupList[j + 1].fileName);
            }
            backupCount--;

            // Increment backup file count
            backupFiles++;
            break;
        }
    }

    if (foundMatch) {
        // Save updated file lists
        saveFileList("Processed1.txt", processed1List, processed1Count);
        saveFileList("Backup.txt", backupList, backupCount);

        // Update the backup file count
        updateFileCounts();

        // Start the animation
        glutTimerFunc(30, updateAnimation, 0);
    }

    free(processed1List);
    free(backupList);
}

void checkAndProcessBackup(int value) {
    compareAndUpdateBackup();
    glutTimerFunc(CHECK_INTERVAL, checkAndProcessBackup, 0);
}



void processFiles() {
    if (processedFileCount == 0 || homeFileCount == 0) {
        // No files left to process
        isPaperVisible = false;
        updateFileCounts(); // Update counts
        glutPostRedisplay();
        return;
    }

    // Get the first file in Processed.txt
    char *processedFirstFile = processedFileList[0].fileName;
    bool fileMoved = false;

    // Check against all files in home.txt
    for (int j = 0; j < homeFileCount; j++) {
        if (strcmp(processedFirstFile, homeFileList[j].fileName) == 0) {
            // Save the current file name for animation
            strcpy(currentFile, homeFileList[j].fileName);

            // Debugging: Print the file being processed
            printf("Processing file: %s\n", currentFile);

            // Move file to Processed directory (animation)
            isPaperVisible = true;
            paperX = -0.2f; // Reset paper position
            paperY = 0.6f;  // Reset paper position
            stage = 0;      // Start animation stage
            glutTimerFunc(30, updateAnimation, 0);

            // Remove from home.txt
            for (int k = j; k < homeFileCount - 1; k++) {
                strcpy(homeFileList[k].fileName, homeFileList[k + 1].fileName);
            }
            homeFileCount--;

            // Save to Processed1.txt
            FILE *processed1File = fopen("Processed1.txt", "a");
            if (!processed1File) {
                printf("Error: Unable to open Processed1.txt\n");
                return;
            }
            fprintf(processed1File, "%s\n", currentFile);
            fclose(processed1File);

            fileMoved = true;
            break; // Exit loop after processing the file
        }
    }

    if (fileMoved) {
        // Remove the first line from Processed.txt
        for (int i = 0; i < processedFileCount - 1; i++) {
            strcpy(processedFileList[i].fileName, processedFileList[i + 1].fileName);
        }
        processedFileCount--;

        // Update the file lists
        saveFileList("home.txt", homeFileList, homeFileCount);
        saveFileList("Processed.txt", processedFileList, processedFileCount);

        // Update file counts after processing
        updateFileCounts();

        // Schedule the next file processing after animation
        glutTimerFunc(1000, processFilesWrapper, 0);
    } else {
        // No matching file found, hide the paper
        isPaperVisible = false;
        updateFileCounts(); // Update counts
    }

    glutPostRedisplay();

    compareAndMoveFiles();
    compareAndUpdateBackup();
    compareAndUpdateDelete();
    

}

void compareAndUpdateDelete() {
    FileList *deleteList = NULL;
    FileList *backup1List = NULL;

    int deleteCount = readFileList("delete.txt", &deleteList);
    int backup1Count = readFileList("Backup1.txt", &backup1List);

    if (deleteCount == 0 || backup1Count == 0) {
        free(deleteList);
        free(backup1List);
        return;
    }

    char *firstDeleteFile = deleteList[0].fileName;
    bool foundMatch = false;

    for (int i = 0; i < backup1Count; i++) {
        if (strcmp(firstDeleteFile, backup1List[i].fileName) == 0) {
            foundMatch = true;

            // Set up animation for Backup to Recycle Bin
            strcpy(currentFile, firstDeleteFile);
            isPaperVisible = true;
            paperX = -0.7f; // Starting position from Backup folder
            paperY = -0.1f; // Adjust as per the Backup folder's position
            stage = 3;      // New stage for Backup to Recycle Bin

            // Write matched file to delete1.txt
            FILE *delete1File = fopen("delete1.txt", "a");
            if (!delete1File) {
                printf("Error: Unable to open delete1.txt\n");
                break;
            }
            fprintf(delete1File, "%s\n", firstDeleteFile);
            fclose(delete1File);

            // Remove from delete.txt
            for (int j = 0; j < deleteCount - 1; j++) {
                strcpy(deleteList[j].fileName, deleteList[j + 1].fileName);
            }
            deleteCount--;

            // Remove from Backup1.txt
            for (int j = i; j < backup1Count - 1; j++) {
                strcpy(backup1List[j].fileName, backup1List[j + 1].fileName);
            }
            backup1Count--;

            break;
        }
    }

    if (foundMatch) {
        saveFileList("delete.txt", deleteList, deleteCount);
        saveFileList("Backup1.txt", backup1List, backup1Count);

        updateFileCounts(); // Update counts after changes

        // Start the animation
        glutTimerFunc(30, updateAnimation, 0);
    }


    free(deleteList);
    free(backup1List);
}

// Add this function to your update or processing loop
void checkAndProcessDelete(int value) {
    compareAndUpdateDelete();
    glutTimerFunc(CHECK_INTERVAL, checkAndProcessDelete, 0);
}

void compareAndMoveFiles() {
    int unprocessedCount = readFileList("UnProcessed.txt", &unProcessedFileList);
    int homeCount = readFileList("home.txt", &homeFileList);
    int i, j;
    
    // Loop through all files in UnProcessed.txt
    for (i = 0; i < unprocessedCount; i++) {
        char *unprocessedFileName = unProcessedFileList[i].fileName;

        // Check each file in home.txt against the current file in UnProcessed.txt
        for (j = 0; j < homeCount; j++) {
            if (strcmp(unprocessedFileName, homeFileList[j].fileName) == 0) {
                // File found in both UnProcessed.txt and home.txt, move it
                printf("Found matching file: %s\n", unprocessedFileName);
                
                // Remove the file from home.txt and UnProcessed.txt, then save it in UnProcessed1.txt
                saveToUnProcessed1(unprocessedFileName);  // Save to UnProcessed1.txt
                moveFile(unprocessedFileName); // Logic to move file from Home to UnProcessed folder

                // Remove from home.txt
                removeFileFromList(&homeFileList, &homeCount, homeFileList[j]);

                // Remove from UnProcessed.txt
                removeFileFromList(&unProcessedFileList, &unprocessedCount, unProcessedFileList[i]);

                // Update file lists
                saveFileList("home.txt", homeFileList, homeCount);
                saveFileList("UnProcessed.txt", unProcessedFileList, unprocessedCount);
                
                break; // File found and processed, no need to continue checking this file
            }
        }
    }
}

void removeFileFromList(FileList **fileList, int *fileCount, FileList fileToRemove) {
    int i, j;
    for (i = 0; i < *fileCount; i++) {
        if (strcmp((*fileList)[i].fileName, fileToRemove.fileName) == 0) {
            // Shift remaining files to remove the matched file
            for (j = i; j < *fileCount - 1; j++) {
                (*fileList)[j] = (*fileList)[j + 1];
            }
            (*fileCount)--; // Decrease file count
            break;
        }
    }
}

void moveFile(const char *fileName) {
    char homeFilePath[MAX_FILENAME_LEN];
    char unProcessedFilePath[MAX_FILENAME_LEN];

    // Construct file paths (assumes files are in respective folders)
    snprintf(homeFilePath, sizeof(homeFilePath), "Home/%s", fileName);
    snprintf(unProcessedFilePath, sizeof(unProcessedFilePath), "UnProcessed/%s", fileName);

    // Use rename to move the file (you can replace this with actual logic if required)
    if (rename(homeFilePath, unProcessedFilePath) != 0) {
        printf("Error moving file: %s\n", fileName);
    }
}

void compareAndDeleteFiles() {
    int unprocessedCount = readFileList("UnProcessed.txt", &unProcessedFileList);
    int homeCount = readFileList("home.txt", &homeFileList);
    int i, j;

    // Loop through files in home.txt
    for (i = 0; i < homeCount; i++) {
        char *homeFileName = homeFileList[i].fileName;

        // Compare with files in UnProcessed.txt
        for (j = 0; j < unprocessedCount; j++) {
            if (strcmp(homeFileName, unProcessedFileList[j].fileName) == 0) {
                // File found in both lists
                printf("Found matching file: %s\n", homeFileName);

                // Save to UnProcessed1.txt
                saveToUnProcessed1(homeFileName);

                // Remove from home.txt and UnProcessed.txt
                removeFileFromList(&homeFileList, &homeCount, homeFileList[i]);
                removeFileFromList(&unProcessedFileList, &unprocessedCount, unProcessedFileList[j]);

                // Update file lists
                saveFileList("home.txt", homeFileList, homeCount);
                saveFileList("UnProcessed.txt", unProcessedFileList, unprocessedCount);

                // Trigger animation for file movement
                strcpy(currentFile, homeFileName);
                isPaperVisible = true;
                paperX = -0.2f; // Starting position of paper in Home folder
                paperY = 0.6f;
                stage = 0; // Stage for Home to UnProcessed animation

                glutTimerFunc(30, updateAnimation, 0);

                return; // Exit after processing the first match
            }
        }
    }
}
void saveToUnProcessed1(const char *fileName) {
    FILE *unProcessed1File = fopen("UnProcessed1.txt", "a"); // Append mode
    if (!unProcessed1File) {
        printf("Error: Unable to open UnProcessed1.txt\n");
        return;
    }
    fprintf(unProcessed1File, "%s\n", fileName);
    fclose(unProcessed1File);
}


void finalizeFileMove(int value) {
    FILE *processed1File = fopen("Processed1.txt", "r+");  // Open in read-write mode
    if (!processed1File) {
        printf("Error: Unable to open Processed1.txt\n");
        return;
    }

    // First, let's check if the file already exists in Processed1.txt
    bool fileExists = false;
    char line[MAX_FILENAME_LEN];

    // Look for the current file in Processed1.txt
    while (fgets(line, sizeof(line), processed1File)) {
        if (strcmp(line, currentFile) == 0) {
            fileExists = true;
            break;
        }
    }

    if (!fileExists) {
        // Append the moved file to Processed1.txt if it doesn't exist
        fseek(processed1File, 0, SEEK_END); // Move to the end of the file
        fprintf(processed1File, "%s\n", currentFile);
        fclose(processed1File);  // Close the file after writing
    } else {
        printf("File already processed: %s\n", currentFile);
        fclose(processed1File);
    }

    // Update counts after finalizing file move
    updateFileCounts();  // Update processed files count

    // Remove the file from home.txt
    for (int j = 0; j < homeFileCount; j++) {
        if (strcmp(currentFile, homeFileList[j].fileName) == 0) {
            for (int k = j; k < homeFileCount - 1; k++) {
                strcpy(homeFileList[k].fileName, homeFileList[k + 1].fileName);
            }
            homeFileCount--;
            break;
        }
    }

    // Remove the file from Processed.txt
    for (int i = 0; i < processedFileCount; i++) {
        if (strcmp(currentFile, processedFileList[i].fileName) == 0) {
            for (int k = i; k < processedFileCount - 1; k++) {
                strcpy(processedFileList[k].fileName, processedFileList[k + 1].fileName);
            }
            processedFileCount--;
            break;
        }
    }

    // Save the updated lists to their respective files
    saveFileList("home.txt", homeFileList, homeFileCount);
    saveFileList("Processed.txt", processedFileList, processedFileCount);

    // Clear the current file and hide the paper
    strcpy(currentFile, "");
    isPaperVisible = false;

    // Trigger the next file processing
    glutTimerFunc(CHECK_INTERVAL, processFilesWrapper, 0);
}


void saveFileList(const char *fileName, FileList *fileList, int fileCount) {
    FILE *file = fopen(fileName, "w");
    if (!file) {
        printf("Error: Unable to open %s for saving\n", fileName);
        return;
    }

    for (int i = 0; i < fileCount; i++) {
        fprintf(file, "%s\n", fileList[i].fileName);
    }

    fclose(file);
}


//function to draw the recycle bin
void drawTable(float startX, float startY) {
    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.8f, 1.0f); // Light blue top
    glVertex2f(startX, startY);
    glVertex2f(startX + 0.88f, startY);
    glColor3f(0.4f, 0.4f, 0.9f); // Darker blue bottom
    glVertex2f(startX + 0.88f, startY - rowHeight);
    glVertex2f(startX, startY - rowHeight);
    glEnd();

    const char *headers[] = {"Generator ID", "File Name", "Rows", "Columns"};
    float headerXPos[] = {startX + 0.0f, startX + 0.24f, startX + 0.48f, startX + 0.68f};

    glColor3f(1.0f, 1.0f, 1.0f); // White text
    for (int i = 0; i < 4; i++) {
        glRasterPos2f(headerXPos[i], startY - rowHeight + 0.03f);
        for (const char *c = headers[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }

    // Draw the rows based on startRow (which is updated by scrollbar movement)
    for (int i = startRow; i < startRow + maxVisibleRows && i < totalDataRows; i++) {
        float rowY = startY - (i - startRow + 1) * rowHeight;

        // Alternate row colors for better readability
        if (i % 2 == 0) {
            glColor3f(0.95f, 0.95f, 0.95f); // Light gray
        } else {
            glColor3f(0.85f, 0.85f, 0.85f); // Darker gray
        }
        glBegin(GL_QUADS);
        glVertex2f(startX, rowY);
        glVertex2f(startX + 0.88f, rowY);
        glVertex2f(startX + 0.88f, rowY - rowHeight);
        glVertex2f(startX, rowY - rowHeight);
        glEnd();

        // Draw row text
        glColor3f(0.0f, 0.0f, 0.0f); // Black text
        char buffer[50];
        float columnXPos[] = {startX + 0.02f, startX + 0.24f, startX + 0.46f, startX + 0.68f};

        snprintf(buffer, sizeof(buffer), "%d", rowData[i].generatorID);
        glRasterPos2f(columnXPos[0], rowY - 0.05f);
        for (const char *c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        glRasterPos2f(columnXPos[1], rowY - 0.05f);
        for (const char *c = rowData[i].fileName; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        snprintf(buffer, sizeof(buffer), "%d", rowData[i].rows);
        glRasterPos2f(columnXPos[2], rowY - 0.05f);
        for (const char *c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        snprintf(buffer, sizeof(buffer), "%d", rowData[i].columns);
        glRasterPos2f(columnXPos[3], rowY - 0.05f);
        for (const char *c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
}
// Function to draw the scrollbar
void readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Unable to open file %s\n", filename);
        return;
    }

    char line[MAX_LINE_LEN];
    if (rowData == NULL) {
        allocatedRows = 10;
        rowData = (RowData *)malloc(allocatedRows * sizeof(RowData));
    }

    int currentRowCount = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0'; // Remove newline

        if (currentRowCount >= allocatedRows) {
            allocatedRows *= 2;
            rowData = (RowData *)realloc(rowData, allocatedRows * sizeof(RowData));
        }

        if (sscanf(line, "%d,%49[^,],%d,%d",
                   &rowData[currentRowCount].generatorID,
                   rowData[currentRowCount].fileName,
                   &rowData[currentRowCount].rows,
                   &rowData[currentRowCount].columns) == 4) {
            currentRowCount++;
        }
    }
    fclose(file);

    if (currentRowCount != totalDataRows) {
        totalDataRows = currentRowCount;
        calculateScrollbarThumbHeight(); // Recalculate scrollbar
        if (startRow + maxVisibleRows > totalDataRows) {
            startRow = totalDataRows > maxVisibleRows ? totalDataRows - maxVisibleRows : 0;
        }
        glutPostRedisplay(); // Force table redraw
    }
}
// Function to check if the file has been updated
void checkFileForUpdates(int value) {
    struct stat fileStat;
    if (stat(filename, &fileStat) == 0) {
        if (fileStat.st_mtime > lastModifiedTime) {
            lastModifiedTime = fileStat.st_mtime;
            readFile(filename);
        }
    }

    // Update Home file count dynamically
    homeFiles = countFilesInHome("home.txt");

    glutTimerFunc(CHECK_INTERVAL, checkFileForUpdates, 0);
}

// Free allocated memory
void freeRowData() {
    if (rowData != NULL) {
        free(rowData);
        rowData = NULL;
    }
}
// Function to calculate scrollbar thumb height dynamically
void calculateScrollbarThumbHeight() {
    if (totalDataRows <= maxVisibleRows) {
        scrollbarThumbHeight = 0.0f;  // No scrollbar if rows are less than or equal to visible rows
    } else {
        float visibleFraction = (float)maxVisibleRows / totalDataRows;
        scrollbarThumbHeight = (scrollbarYStart - scrollbarYEnd) * visibleFraction;

        if (scrollbarThumbHeight > (scrollbarYStart - scrollbarYEnd)) {
            scrollbarThumbHeight = scrollbarYStart - scrollbarYEnd; // Clamp to maximum height
        }
    }
}

// Function to draw the scrollbar
void drawScrollbar() {
    glColor3f(0.8f, 0.8f, 0.8f); // Light gray track
    glBegin(GL_QUADS);
    glVertex2f(scrollbarX, scrollbarYStart);
    glVertex2f(scrollbarX + scrollbarWidth, scrollbarYStart);
    glVertex2f(scrollbarX + scrollbarWidth, scrollbarYEnd);
    glVertex2f(scrollbarX, scrollbarYEnd);
    glEnd();

    // Draw the scrollbar thumb
    if (scrollbarThumbHeight > 0.0f) {  // Only draw if there is a scrollbar
        glColor3f(0.4f, 0.4f, 0.4f); // Darker gray thumb
        glBegin(GL_QUADS);
        glVertex2f(scrollbarX, scrollbarThumbY);  // Top of the thumb
        glVertex2f(scrollbarX + scrollbarWidth, scrollbarThumbY);
        glVertex2f(scrollbarX + scrollbarWidth, scrollbarThumbY - scrollbarThumbHeight); // Bottom of the thumb
        glVertex2f(scrollbarX, scrollbarThumbY - scrollbarThumbHeight);
        glEnd();
    }
}
void setScrollbarToBottom() {
    // Ensure that scrollbar starts at the bottom to show the most recent rows
    scrollbarThumbY = scrollbarYStart;  // Position thumb at the bottom of the scrollbar
    startRow = totalDataRows - maxVisibleRows;  // Start from the last row

    if (startRow < 0) startRow = 0;  // Ensure valid range for startRow

    // Update the scrollbar thumb height and redraw
    calculateScrollbarThumbHeight();
    glutPostRedisplay();
}

// Function to update the scrollbar thumb position
void updateScrollbarThumb(float deltaY) {
    scrollbarThumbY += deltaY;
    if (scrollbarThumbY > scrollbarYStart) scrollbarThumbY = scrollbarYStart;
    if (scrollbarThumbY < scrollbarYEnd + scrollbarThumbHeight) scrollbarThumbY = scrollbarYEnd + scrollbarThumbHeight;

    float scrollFraction = (scrollbarYStart - scrollbarThumbY) / (scrollbarYStart - scrollbarYEnd - scrollbarThumbHeight);
    startRow = (int)(scrollFraction * (totalDataRows - maxVisibleRows));
    if (startRow < 0) startRow = 0;
    if (startRow + maxVisibleRows > totalDataRows) startRow = totalDataRows - maxVisibleRows;

    glutPostRedisplay();
}
// function to handle mouse wheel scrolling
void mouseWheelCallback(int button, int direction, int x, int y) {
    // Adjust scroll direction based on wheel movement
    float deltaY = (direction > 0) ? 0.05f : -0.05f;
    
    updateScrollbarThumb(deltaY * (scrollbarYStart - scrollbarYEnd));  // Update thumb position based on scroll
    glutPostRedisplay();  // Redraw the screen
}

/////////////////////////////////

void drawTextt(float x, float y, const char* text, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}



///////////////////////////////
int countFilesInHome(const char *homeFile) {
    FILE *file = fopen(homeFile, "r");
    if (!file) {
        printf("Error: Unable to open file %s\n", homeFile);
        return 0;
    }

    int fileCount = 0;
    char line[MAX_FILENAME_LEN];
    while (fgets(line, sizeof(line), file)) {
        // Count non-empty lines
        if (line[0] != '\0' && line[0] != '\n') {
            fileCount++;
        }
    }

    fclose(file);
    return fileCount;
}



// Function to read a file into a list of filenames
int readFileList(const char *fileName, FileList **fileList) {
    FILE *file = fopen(fileName, "r");
    if (!file) return 0;
    
    int count = 0;
    char line[MAX_FILENAME_LEN];
    *fileList = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0'; // Remove newline character
        if (strlen(line) > 0) {
            *fileList = realloc(*fileList, (count + 1) * sizeof(FileList));
            strncpy((*fileList)[count].fileName, line, MAX_FILENAME_LEN);
            count++;
        }
    }
    
    fclose(file);
    return count;
}
//////////////////////////////
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawBackground();

    // Draw folders
    drawFolder(-0.2f, 0.6f);
    drawFolder(-0.7f, 0.6f);
    drawFolder(-0.2f, -0.1f);
    drawFolder(-0.7f, -0.1f);
    drawRecycleBin(-0.7f, -0.6f);

    // Draw the moving file
    drawPaperFile(fileX, fileY);

    // Draw connecting lines
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.2f, 0.55f);
    glVertex2f(-0.7f, 0.55f);
    glVertex2f(-0.2f, 0.55f);
    glVertex2f(-0.2f, -0.1f);
    glVertex2f(-0.7f, 0.55f);
    glVertex2f(-0.7f, -0.1f);
    glVertex2f(-0.7f, -0.15f);
    glVertex2f(-0.7f, -0.55f);
    glEnd();

    // Draw folder labels
    drawText(-0.2f, 0.65f, "Home", 0.0f, 0.0f, 0.0f);
    drawText(-0.75f, 0.65f, "Processed", 0.0f, 0.0f, 0.0f);
    drawText(-0.25f, -0.05f, "Unprocessed", 0.0f, 0.0f, 0.0f);
    drawText(-0.75f, -0.05f, "Backup", 0.0f, 0.0f, 0.0f);
    drawText(-0.8f, -0.45f, "Recycle Bin", 0.0f, 0.0f, 0.0f);

        // Draw the file counts inside circles at folder corners
    drawFileCountInCircle(-0.05f, 0.75f, homeFiles);           // Home folder top-right corner
    drawFileCountInCircle(-0.55f, 0.75f, processedFiles);    // Processed folder top-right corner
    drawFileCountInCircle(-0.05f, 0.05f, unprocessedFiles);   // Unprocessed folder top-right corner
    drawFileCountInCircle(-0.55f, 0.05f, countNonEmptyLinesInBackup1("Backup1.txt"));
      // Backup folder top-right corner
    drawFileCountInCircle(-0.55f, -0.37f, recycleBinFiles); // Recycle Bin top-right corner

    // Draw the table
    drawTable(0.1f, 0.65f);

    // Draw the rectangle below the table
    // Draw the scrollbar
    drawScrollbar();

    glFlush();
}

void mouseWheelSimulated(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == 3) { // Scroll up
            updateScrollbarThumb(0.05f * (scrollbarYStart - scrollbarYEnd));
        } else if (button == 4) { // Scroll down
            updateScrollbarThumb(-0.05f * (scrollbarYStart - scrollbarYEnd));
        }
        glutPostRedisplay();
    }
}


void initGraphics(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("File Animation");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    homeFiles = readFileList("home.txt", &homeFileList);
    processedFiles = readFileList("Processed.txt", &processedFileList);

    readFile(filename); 

    calculateScrollbarThumbHeight();

    glutDisplayFunc(display);
    glutTimerFunc(CHECK_INTERVAL, checkFileForUpdates, 0);
    glutTimerFunc(CHECK_INTERVAL, checkAndProcessBackup, 0);
    glutTimerFunc(CHECK_INTERVAL, checkAndProcessDelete, 0);
        setScrollbarToBottom();

    glutMouseFunc(mouseWheelSimulated);
    glutDisplayFunc(display);


    glutMainLoop();
}
