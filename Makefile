# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -fconstexpr-steps=30000000 -fconstexpr-backtrace-limit=0

# Output executable name
TARGET = my_chess.exe

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Source files (now inside src/)
SRCS = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/nnue/*.cpp)

# Object files (stored in obj/)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Default rule: build the program
all: $(TARGET)

# Link object files into the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cpp into .o inside obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/nnue
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up compiled files
clean:
	rm -rf $(TARGET) $(OBJ_DIR)
