.PHONY: clean

CXX ?= g++
CXXFLAGS ?= -std=c++11 -march=native -O3
# -g -ggdb -fsanitize=address

all: primegen

primegen: primegen.cpp primegen.hpp
	$(CXX) $(CXXFLAGS) -o $@ primegen.cpp

check: primegen
	./primegen 1 1000000000 -s > .test.txt
	test `cat .test.txt | cut -d' ' -f1` = "count=50847534"
	test `cat .test.txt | cut -d' ' -f2` = "sum=24739512092254535"
	rm .test.txt
	echo "OK"
	
clean:
	rm primegen
