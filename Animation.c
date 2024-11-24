#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>



// Function to draw text on the screen
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

void drawPaperFile(float xOffset, float yOffset) {
    glColor3f(1.0f, 1.0f, 1.0f);  // White color for the paper
    glBegin(GL_POLYGON);
    glVertex2f(-0.05f + xOffset, -0.1f + yOffset); // Left-bottom corner
    glVertex2f(0.05f + xOffset, -0.1f + yOffset);  // Right-bottom corner
    glVertex2f(0.05f + xOffset, 0.1f + yOffset);   // Right-top corner
    glVertex2f(-0.05f + xOffset, 0.1f + yOffset);  // Left-top corner
    glEnd();

    // Draw lines inside the file (like paper lines)
    glColor3f(0.0f, 0.0f, 0.0f);  // Black color for the lines
    float lineSpacing = 0.015f;
    for (float i = 0.05f; i > -0.05f; i -= lineSpacing) {
        glBegin(GL_LINES);
        glVertex2f(-0.04f + xOffset, i + yOffset); // Left side of the line
        glVertex2f(0.04f + xOffset, i + yOffset);  // Right side of the line
        glEnd();
    }
}

// Function to draw a file
void drawFile(float xOffset, float yOffset) {
    glColor3f(0.2f, 0.6f, 1.0f);  // Blue color for the file
    glBegin(GL_POLYGON);
    glVertex2f(-0.05f + xOffset, -0.1f + yOffset); // Left-bottom corner
    glVertex2f(0.05f + xOffset, -0.1f + yOffset);  // Right-bottom corner
    glVertex2f(0.05f + xOffset, 0.05f + yOffset);  // Right-top corner
    glVertex2f(-0.05f + xOffset, 0.05f + yOffset); // Left-top corner
    glEnd();
}


// Function to draw a smaller folder
void drawFolder(float xOffset, float yOffset) {
    // Draw the main body of the folder
    glColor3f(1.0f, 0.9f, 0.0f);  // Yellow color
    glBegin(GL_POLYGON);
    glVertex2f(-0.15f + xOffset, -0.1f + yOffset);
    glVertex2f(0.15f + xOffset, -0.1f + yOffset);
    glVertex2f(0.15f + xOffset, 0.05f + yOffset);
    glVertex2f(-0.15f + xOffset, 0.05f + yOffset);
    glEnd();

    // Draw the shadow for the main body
    glColor3f(0.8f, 0.7f, 0.0f);  // Darker yellow color for shadow
    glBegin(GL_POLYGON);
    glVertex2f(-0.15f + xOffset, -0.1f + yOffset);
    glVertex2f(-0.1f + xOffset, -0.15f + yOffset);
    glVertex2f(0.1f + xOffset, -0.15f + yOffset);
    glVertex2f(0.15f + xOffset, -0.1f + yOffset);
    glEnd();

    // Draw the top flap of the folder
    glColor3f(1.0f, 0.8f, 0.0f);  // Slightly lighter yellow
    glBegin(GL_POLYGON);
    glVertex2f(-0.1f + xOffset, 0.05f + yOffset);
    glVertex2f(0.05f + xOffset, 0.15f + yOffset);
    glVertex2f(0.15f + xOffset, 0.15f + yOffset);
    glVertex2f(0.15f + xOffset, 0.05f + yOffset);
    glEnd();

    // Draw the shadow for the top flap
    glColor3f(0.9f, 0.7f, 0.0f);  // Darker yellow color for shadow
    glBegin(GL_POLYGON);
    glVertex2f(-0.1f + xOffset, 0.05f + yOffset);
    glVertex2f(-0.05f + xOffset, 0.0f + yOffset);
    glVertex2f(0.15f + xOffset, 0.0f + yOffset);
    glVertex2f(0.15f + xOffset, 0.05f + yOffset);
    glEnd();
}

// Function to draw a realistic Recycle Bin
void drawRecycleBin(float xOffset, float yOffset) {
    // Draw the main body of the Recycle Bin (larger and with rounded edges)
    glColor3f(0.7f, 0.7f, 0.7f);  // Grey color
    glBegin(GL_POLYGON);
    glVertex2f(-0.2f + xOffset, -0.2f + yOffset);
    glVertex2f(0.2f + xOffset, -0.2f + yOffset);
    glVertex2f(0.15f + xOffset, 0.2f + yOffset);
    glVertex2f(-0.15f + xOffset, 0.2f + yOffset);
    glEnd();

    // Draw the top edge of the Recycle Bin (lip)
    glColor3f(0.6f, 0.6f, 0.6f);  // Darker grey
    glBegin(GL_POLYGON);
    glVertex2f(-0.2f + xOffset, -0.2f + yOffset);
    glVertex2f(0.2f + xOffset, -0.2f + yOffset);
    glVertex2f(0.18f + xOffset, -0.15f + yOffset);
    glVertex2f(-0.18f + xOffset, -0.15f + yOffset);
    glEnd();

    // Draw a recycling symbol in the middle
    glColor3f(0.0f, 0.5f, 0.0f);  // Green color
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f + xOffset, 0.0f + yOffset);  // Top point of the triangle
    glVertex2f(-0.05f + xOffset, -0.05f + yOffset);  // Bottom-left point
    glVertex2f(0.05f + xOffset, -0.05f + yOffset);  // Bottom-right point
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f + xOffset, 0.05f + yOffset);  // Top point of the triangle
    glVertex2f(-0.05f + xOffset, 0.0f + yOffset);  // Bottom-left point
    glVertex2f(0.05f + xOffset, 0.0f + yOffset);  // Bottom-right point
    glEnd();
}

// Function to draw a line connecting two points
void drawLine(float x1, float y1, float x2, float y2) {
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

// Function to adjust the display function to place the file above the "Home" directory
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Draw the file icons (folders)
    drawFolder(0.0f, 0.6f);  // Home directory
    drawFolder(-0.5f, 0.6f); // Processed
    drawFolder(0.0f, -0.1f); // Unprocessed
    drawFolder(-0.5f, -0.1f); // Backup
    drawRecycleBin(-0.5f, -0.6f); // Larger Recycle Bin

    // Draw the file above the Home directory (representing a file with lines like paper)
    drawPaperFile(0.0f, 0.6f);  // Paper file above Home directory

    // Draw lines connecting the icons
    drawLine(0.0f, 0.55f, -0.5f, 0.55f);  // Home -> Processed
    drawLine(0.0f, 0.55f, 0.0f, -0.1f);   // Home -> Unprocessed
    drawLine(-0.5f, 0.55f, -0.5f, -0.1f); // Processed -> Backup
    drawLine(-0.5f, -0.15f, -0.5f, -0.55f); // Backup -> Recycle Bin

    // Display text for directory names
    drawText(0.0f, 0.65f, "Home");          // Home directory
    drawText(-0.55f, 0.65f, "Processed");    // Processed directory
    drawText(-0.1f, -0.05f, "Unprocessed");  // Unprocessed directory
    drawText(-0.5f, -0.05f, "Backup");      // Backup directory
    drawText(-0.6f, -0.45f, "Recycle Bin"); // Recycle Bin

    // Display text for the timer and directory listings
    drawText(-0.9f, 0.9f, "Timer: 00:00");
    drawText(0.4f, 0.9f, "#files in the Home directory:");
    drawText(0.4f, 0.8f, "#files in the Processed directory:");
    drawText(0.4f, 0.7f, "#files in the Unprocessed directory:");
    drawText(0.4f, 0.6f, "#files in the Backup directory:");
    drawText(0.4f, 0.5f, "#files in the Recycle Bin:");

    glFlush();
}
// int main(int argc, char** argv) {
//     glutInit(&argc, argv);
//     glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
//     glutInitWindowSize(1000, 800);
//     glutCreateWindow("Hierarchical File Structure");

//     glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background
//     gluOrtho2D(-1.0, 1.0, -1.0, 1.0);  // Set coordinate system

//     glutDisplayFunc(display);
//     glutMainLoop();

//     return 0;
// }