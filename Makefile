CC = gcc
CFLAGS = -Wall -g
LDFLAGS = $(shell pkg-config --cflags --libs readline) $(shell pkg-config --cflags --libs python3)

# Check for required packages (readline and python3)
PKG_CONFIG = pkg-config

# Targets
TARGET = t-shell
SRC = t-shell.c
OBJ = t-shell.o

# Install required packages (for Ubuntu/Debian systems)
install:
	@echo "Installing dependencies..."
	@sudo apt-get update
	@sudo apt-get install -y \
		libreadline-dev \
		python3-pygments \
		python3 \
		pkg-config

# Compile the program
$(TARGET): $(OBJ)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)
	@echo "Compilation successful! You can now run the program using './$(TARGET)'"

# Compile the source code into an object file
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Default target to build everything
all: $(TARGET)

.PHONY: install clean run all
