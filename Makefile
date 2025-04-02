CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src
LDFLAGS = 

# Find all .cpp files under src.
SRC := $(shell find src -type f -name '*.cpp')
# Convert each src/xxx.cpp into build/xxx.o
OBJ := $(patsubst src/%, build/%, $(SRC:.cpp=.o))
EXEC = slamzehausdown

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Pattern rule: for any file src/anything.cpp, compile to build/anything.o
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(EXEC)
