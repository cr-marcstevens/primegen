.PHONY: all clean check

CXX ?= g++
CXXFLAGS ?= -std=c++11 -march=native -O3
# -g -ggdb -fsanitize=address

all: primegen almostprimecount

primegen: primegen.cpp primegen.hpp
	$(CXX) $(CXXFLAGS) -o $@ primegen.cpp

almostprimecount: almostprimecount.cpp primegen.hpp
	$(CXX) $(CXXFLAGS) -o $@ almostprimecount.cpp

check: primegencheck almostprimecountcheck

primegencheck: primegen
	@echo "Running simple primegen test..."
	@./primegen 1 1000000000 -s > .test.txt
	@test `cat .test.txt | cut -d' ' -f1` = "count=50847534"
	@test `cat .test.txt | cut -d' ' -f2` = "sum=24739512092254535"
	@rm .test.txt
	@echo "OK"

almostprimecountcheck: almostprimecount
	@echo "Running simple almostprimecount test..."
	@./almostprimecount 27 > .test.txt
	@test `tail -n1 .test.txt | tr -dc "[:alnum:] " | sha256sum - | cut -d' ' -f1` = "7bca6e3dea44e59f57544e01007c37e993e7e1cdc2a2da0c11054ed31d0593a4"
	@rm .test.txt
	@echo "OK"

clean:
	rm primegen
