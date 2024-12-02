#ifdef __APPLE_CC__
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

// Adjust the scrollbar positions
float scrollbarX = 0.98f; // X position of the scrollbar (closer to the table)
float scrollbarYStart = 0.53f; // Start Y position of the scrollbar (below the first row)
float scrollbarYEnd = -0.187f; // Bottom Y position of the scrollbar
float scrollbarWidth = 0.03f; // Width of the scrollbar
float scrollbarThumbY = 0.53f; // Initial Y position of the scrollbar thumb
float scrollbarThumbHeight; // Dynamically calculated height of the scrollbar thumb

bool draggingScrollbar = false; // Flag to check if the scrollbar is being dragged
float dragStartY = 0.0f; // Starting Y position of the drag
#define CHECK_INTERVAL 1000 // Time interval in milliseconds for checking the file
#define ROW_HEIGHT 0.12f
#define MAX_FILENAME_LEN 50
#define MAX_LINE_LEN 256

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
char filename[MAX_FILENAME_LEN] = "home.txt";
time_t lastModifiedTime = 0;
float windowHeight = 800.0f; // Default window height
// File position variables
float fileX = -0.2f;
float fileY = 0.6f;
int stage = 0; // Animation stage: 0 = Home -> Processed, etc.
// Number of rows to display at a time
#define ROWS_PER_PAGE 6
// Timer variable
float timer = 0.0f;
// File counters for each folder
int homeFiles = 1;
int processedFiles = 0;
int unprocessedFiles = 0;
int backupFiles = 0;
int recycleBinFiles = 0;
#define MAX_ROWS 100
// Global variable to control the scroll offset
float scrollOffset = 0.0f;
const int totalRows = 6;        // Total number of rows in your table
const float tableHeight = rowHeight * totalRows; // The height of all rows combined
// Adjust this to set the number of visible rows
const int VISIBLE_ROWS = 6;

// Function to draw text on the screen
void drawText(float x, float y, const char* text, float r, float g, float b) {
    glColor3f(r, g, b); // Set text color
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}
// Function to draw a moving file
void drawPaperFile(float xOffset, float yOffset) {
    glColor3f(1.0f, 1.0f, 1.0f); // White
    glBegin(GL_POLYGON);
    glVertex2f(-0.05f + xOffset, -0.1f + yOffset);
    glVertex2f(0.05f + xOffset, -0.1f + yOffset);
    glVertex2f(0.05f + xOffset, 0.1f + yOffset);
    glVertex2f(-0.05f + xOffset, 0.1f + yOffset);
    glEnd();

    // Draw lines inside the file (paper lines)
    glColor3f(0.0f, 0.0f, 0.0f); // Black
    float lineSpacing = 0.015f;
    for (float i = 0.05f; i > -0.05f; i -= lineSpacing) {
        glBegin(GL_LINES);
        glVertex2f(-0.04f + xOffset, i + yOffset);
        glVertex2f(0.04f + xOffset, i + yOffset);
        glEnd();
    }
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
void updateAnimation(int value) {
    timer += 0.03f; // Increment timer

    if (stage == 0) { // Home -> Processed
        // Move file towards the processed folder on the x-axis
        if (fileX > -0.7f) { 
            fileX -= 0.03f; // Move file leftward towards the processed folder
        } else {
            stage = 1; // File has reached the Processed folder
            homeFiles--;   // Decrease home folder count
            processedFiles++; // Increase processed folder count
        }
    }

    // File movement should not reset or interfere with the scrolling rows.
    // So we only update the file position, not the table view (startRow).
    glutPostRedisplay();  // Request a redraw
    glutTimerFunc(30, updateAnimation, 0); // Continuously update the animation
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
// Function to display the file count inside a circle
void drawFileCountInCircle(float x, float y, int fileCount) {
    // Draw circle in the top-right corner of each folder with a green color
    glColor3f(0.0f, 1.0f, 0.0f);  // Green color for the circle outline
    drawCircle(x, y, 0.05f);      // Radius of the circle

    // Fill the circle with a green color
    glColor3f(0.0f, 1.0f, 0.0f);  // Green color for the circle fill
    drawCircle(x, y, 0.05f);      // Same radius for filling

    // Draw the file count text inside the circle with white color
    char countText[10];
    snprintf(countText, sizeof(countText), "%d", fileCount);
    glColor3f(1.0f, 1.0f, 1.0f);  // White color for the text

    // Adjust the position of the text to move it a little more to the left
    glRasterPos2f(x - 0.01f, y - 0.03f);  // Move text slightly to the left
    for (const char* c = countText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); // Smaller font
    }
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
            readFile(filename); // Reload file if modified
        }
    }
    glutTimerFunc(CHECK_INTERVAL, checkFileForUpdates, 0); // Reschedule
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
// Function to handle mouse drag events
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
    drawPaperFile(fileX, fileY); // Draw the moving file at the updated fileX position

    // Draw connecting lines (unchanged)
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

    // Draw folder labels (unchanged)
    drawText(-0.2f, 0.65f, "Home", 0.0f, 0.0f, 0.0f);
    drawText(-0.75f, 0.65f, "Processed", 0.0f, 0.0f, 0.0f);
    drawText(-0.25f, -0.05f, "Unprocessed", 0.0f, 0.0f, 0.0f);
    drawText(-0.75f, -0.05f, "Backup", 0.0f, 0.0f, 0.0f);
    drawText(-0.8f, -0.45f, "Recycle Bin", 0.0f, 0.0f, 0.0f);

    // Draw the table using the current `startRow`
    drawTable(0.1f, 0.65f);

    // Draw the scrollbar
    drawScrollbar();

    glFlush();
}






// Function to initialize graphics
void initGraphics(int argc, char *argv[]){ // Add the function prototype 
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("Table with Scrollbar");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // Set background color to white
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);     // Set the orthogonal projection

    readFile(filename);  // Load the file data

    // Calculate the initial scrollbar thumb height
    calculateScrollbarThumbHeight();

    // Set the callback functions
    glutDisplayFunc(display);
    glutTimerFunc(CHECK_INTERVAL, checkFileForUpdates, 0); // Check for file updates periodically
    glutMouseWheelFunc(mouseWheelCallback); // Mouse wheel callback for scrolling

    // Start the file movement animation
    updateAnimation(0);  // Initial call to updateAnimation to start the file movement

    glutMainLoop();  // Start the GLUT main loop
    free(rowData);   // Free allocated memory after the loop ends


}
