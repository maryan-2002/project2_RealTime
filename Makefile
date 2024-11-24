# Define the compiler
CC = gcc

# Define compiler flags
CFLAGS = -Wall -Wextra -O2

# Define the source files and the target executable
SRC = main.c  Animation.c generator.c
OBJ = main.o  Animation.o generator.o
TARGET = globel

# Default target to compile the program
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lGL -lGLU -lglut -lpthread -lm  # Link OpenGL, pthread, and math libraries

# Compile source files into object files
main.o: main.c
	$(CC) $(CFLAGS) -c main.c


Animation.o: Animation.c  # Added rule for animation.o
	$(CC) $(CFLAGS) -c Animation.c

generator.o: generator.c
	$(CC) $(CFLAGS) -c generator.c

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up the generated files
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets so Make doesn't confuse them with files
.PHONY: all run clean