CXX=g++
CXXFLAGS=-std=c++11 -march=native -O3

primegen: primegen.cpp primegen.hpp
	$(CXX) $(CXXFLAGS) -o $@ primegen.cpp
