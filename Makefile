# Define the objects needed to compile the program
objects = main.o pipline.o built_in.o external.o tools.o

# Define the compiler to use
CC = gcc

# Define the target executable name
TARGET = hyrShell

# Default rule to build the program
all: $(TARGET)

# Rule for linking the objects into an executable
$(TARGET): $(objects)
	$(CC) -g -o $@ $^ -lreadline

# Rules for compiling individual object files
main.o: main.c
	$(CC) -c main.c

pipline.o: pipline.c pipline.h
	$(CC) -c pipline.c

built_in.o: built_in.c built_in.h
	$(CC) -c built_in.c

external.o: external.c external.h
	$(CC) -c external.c

tools.o: tools.c tools.h
	$(CC) -c tools.c

# Clean rule to remove all object files and the executable
.PHONY: clean
clean:
	rm -f $(TARGET) $(objects)