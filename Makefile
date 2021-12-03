.PHONY: clean

CXX ?= g++
CXXFLAGS ?= -std=c++11 -march=native -O3
# -g -ggdb -fsanitize=address

all: primegen

primegen: primegen.cpp primegen.hpp
	$(CXX) $(CXXFLAGS) -o $@ primegen.cpp

check: primegen
	test "24739512092254535" = `./primegen 1 1000000000 -s` && echo "OK"

clean:
	rm primegen
