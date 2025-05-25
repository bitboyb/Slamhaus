CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDFLAGS = 

# Detect platform and force static linking on Windows (for GitHub Actions or local MSYS2 builds)
ifeq ($(OS),Windows_NT)
    LDFLAGS += -static -static-libgcc -static-libstdc++
endif

# Find all directories in src (for includes)
INCLUDE_DIRS := $(shell find src -type d)
INCLUDES := $(foreach dir, $(INCLUDE_DIRS), -I$(dir))
CXXFLAGS += $(INCLUDES)

# Find all .cpp files under src.
SRC := $(shell find src -type f -name '*.cpp')
# Convert each src/xxx.cpp into build/xxx.o
OBJ := $(patsubst src/%, build/%, $(SRC:.cpp=.o))

EXEC = slamhaus

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Pattern rule: for any file src/anything.cpp, compile to build/anything.o
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(EXEC)
