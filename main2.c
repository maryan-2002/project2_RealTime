#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// File position variables
float fileX = 0.0f;
float fileY = 0.6f;
int stage = 0; // Animation stage: 0 = Home -> Processed, etc.

// Timer variable
float timer = 0.0f;

// File counters for each folder
int homeFiles = 1;
int processedFiles = 0;
int unprocessedFiles = 0;
int backupFiles = 0;
int recycleBinFiles = 0;

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

// Animation update function
void updateAnimation(int value) {
    timer += 0.03f; // Increment timer

    if (stage == 0) { // Home -> Processed
        if (fileX > -0.5f) {
            fileX -= 0.01f; // Move left
        } else {
            stage = 1; // Move to next stage
            homeFiles--;
            processedFiles++;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(30, updateAnimation, 0);
}

// Display function
// Display function
// Function to draw a rectangular box for labels
// Function to draw a rectangular box for labels
void drawLabelBox(float x, float y, float width, float height, float r, float g, float b) {
    glColor3f(r, g, b); // Set box color
    glBegin(GL_POLYGON);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();
}

// Updated Display Function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawBackground();

    // Draw folders
    drawFolder(0.0f, 0.6f);       // Home folder
    drawFolder(-0.5f, 0.6f);      // Processed folder
    drawFolder(0.0f, -0.1f);      // Unprocessed folder
    drawFolder(-0.5f, -0.1f);     // Backup folder
    drawRecycleBin(-0.5f, -0.6f); // Recycle Bin

    // Draw the moving file
    drawPaperFile(fileX, fileY);

    // Draw connecting lines
    glColor3f(0.0f, 0.0f, 0.0f); // Black
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.55f); glVertex2f(-0.5f, 0.55f); // Home -> Processed
    glVertex2f(0.0f, 0.55f); glVertex2f(0.0f, -0.1f);  // Home -> Unprocessed
    glVertex2f(-0.5f, 0.55f); glVertex2f(-0.5f, -0.1f); // Processed -> Backup
    glVertex2f(-0.5f, -0.15f); glVertex2f(-0.5f, -0.55f); // Backup -> Recycle Bin
    glEnd();

    // Draw folder labels
    drawText(0.0f, 0.65f, "Home", 0.0f, 0.0f, 0.0f);
    drawText(-0.55f, 0.65f, "Processed", 0.0f, 0.0f, 0.0f);
    drawText(-0.1f, -0.05f, "Unprocessed", 0.0f, 0.0f, 0.0f);
    drawText(-0.55f, -0.05f, "Backup", 0.0f, 0.0f, 0.0f);
    drawText(-0.6f, -0.45f, "Recycle Bin", 0.0f, 0.0f, 0.0f);

    // Draw file counters under each folder
    char counterText[50];
    snprintf(counterText, sizeof(counterText), "#files = %d", homeFiles);
    drawText(0.0f, 0.5f, counterText, 0.0f, 0.0f, 1.0f); // Blue color
    snprintf(counterText, sizeof(counterText), "#files = %d", processedFiles);
    drawText(-0.55f, 0.5f, counterText, 0.0f, 0.0f, 1.0f); // Blue color
    snprintf(counterText, sizeof(counterText), "#files = %d", unprocessedFiles);
    drawText(0.0f, -0.2f, counterText, 0.0f, 0.0f, 1.0f); // Blue color
    snprintf(counterText, sizeof(counterText), "#files = %d", backupFiles);
    drawText(-0.55f, -0.2f, counterText, 0.0f, 0.0f, 1.0f); // Blue color
    snprintf(counterText, sizeof(counterText), "#files = %d", recycleBinFiles);
    drawText(-0.55f, -0.65f, counterText, 0.0f, 0.0f, 1.0f); // Blue color

    // Draw label boxes and text
    // Adjusted starting y-coordinate to move the box downward
float boxWidth = 0.7f;  // Width of the box
float boxHeight = 1.0f; // Height of the box
float boxStartY = 0.7f; // Adjusted y-coordinate (moved downward)

drawLabelBox(0.25f, boxStartY, boxWidth, boxHeight, 0.9f, 0.9f, 0.9f); // Main box

// Home Directory Label
char labelText[50];
snprintf(labelText, sizeof(labelText), "#files in the Home directory : %d", homeFiles);
drawText(0.3f, boxStartY - 0.05f, labelText, 0.0f, 0.0f, 0.0f);

// Processed Directory Label
snprintf(labelText, sizeof(labelText), "#files in the Processed directory : %d", processedFiles);
drawText(0.3f, boxStartY - 0.17f, labelText, 0.0f, 0.0f, 0.0f);

// Unprocessed Directory Label
snprintf(labelText, sizeof(labelText), "#files in the Unprocessed directory : %d", unprocessedFiles);
drawText(0.3f, boxStartY - 0.29f, labelText, 0.0f, 0.0f, 0.0f);

// Backup Directory Label
snprintf(labelText, sizeof(labelText), "#files in the Backup directory : %d", backupFiles);
drawText(0.3f, boxStartY - 0.41f, labelText, 0.0f, 0.0f, 0.0f);

// Recycle Bin Directory Label
snprintf(labelText, sizeof(labelText), "#files in the Recycle Bin directory : %d", recycleBinFiles);
drawText(0.3f, boxStartY - 0.53f, labelText, 0.0f, 0.0f, 0.0f);
    // Display timer below the labels with increased space, bigger size, and unique color
    int minutes = (int)timer / 60;
    int seconds = (int)timer % 60;
    char timerText[50];
    snprintf(timerText, sizeof(timerText), "Timer: %02d:%02d", minutes, seconds);

 // Draw the timer below the labels
glColor3f(0.8f, 0.0f, 0.0f); // Set a unique color, e.g., red
glRasterPos2f(0.5f, 0.001f);   // Adjusted Timer position (lower y-coordinate)
for (const char* c = timerText; *c != '\0'; c++) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c); // Bigger font
}


    glFlush();
}

// int main(int argc, char** argv) {
//     //https://github.com/maryan-2002/project2_RealTime.git
//     // glutInit(&argc, argv);
//     // glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
//     // glutInitWindowSize(1000, 800);
//     // glutCreateWindow("Hierarchical File Structure");

//     // glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background
//     // gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

//     // glutDisplayFunc(display);
//     // glutTimerFunc(30, updateAnimation, 0);
//     // glutMainLoop();
//     // return 0;



// }




///////////
