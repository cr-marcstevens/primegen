.PHONY: clean

CXX ?= g++
CXXFLAGS ?= -std=c++11 -march=native -O3

primegen: primegen.cpp primegen.hpp
	$(CXX) $(CXXFLAGS) -o $@ primegen.cpp

check: primegen
	test 1 -eq `./primegen 1000000000 | md5sum | grep "92c178cc5bb85e06366551c0ae7e18f6" | wc -l` && echo "OK"

clean:
	rm primegen
