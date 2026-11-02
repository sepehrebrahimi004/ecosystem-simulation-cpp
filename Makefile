# Build:  make            ->  build/ecosystem
# Test:   make test
# Run:    make run        ->  output/population.csv, output/positions.csv
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -Isrc -Ithird_party -MMD -MP

LIB_SRCS := src/Config.cpp src/Species.cpp src/World.cpp
LIB_OBJS := $(LIB_SRCS:src/%.cpp=build/%.o)

all: build/ecosystem

build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/ecosystem: build/main.o $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

build/test_world: tests/test_world.cpp $(LIB_OBJS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

test: build/test_world
	./build/test_world

run: build/ecosystem
	./build/ecosystem config/default.json --seed 1

build:
	mkdir -p build

clean:
	rm -rf build output

-include $(wildcard build/*.d)

.PHONY: all test run clean
