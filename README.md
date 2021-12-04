# primegen

[![C++ CI](https://github.com/cr-marcstevens/primegen/actions/workflows/cpp.yml/badge.svg)](https://github.com/cr-marcstevens/primegen/actions/workflows/cpp.yml)

- `primegen.hpp`: C++ header-only library to generate small primes using Sieve of Eratosthenes
- `primegen.cpp`: Command line utility

# Getting started

```
git clone https://github.com/cr-marcstevens/primegen
cd primegen
make
make check
./primegen 512     # print primes <= 512
./primegen 256 512 # print primes >= 256, <= 512
```

# Robustness

Various tests have been done to verify correctness:
- check first 1'000'000'000 primes against known good prime generator
- verify number of primes <= N against [wolfram alpha](https://www.wolframalpha.com/input/?i=number+of+primes+%3C+3435973836800), for N up to 3435973836800 (~2^41.6)
- verify sum of primes <= N against [wolfram alpha](https://www.wolframalpha.com/input/?i=sum+of+primes+%3C+2%5E33), for N up to 2^33

# Memory usage

`primegen` generate primes up to a given upper-bound `U` to which it will sieve.
It will use approximately `U/16` bytes of RAM.
TODO: implement segmented version that will use `constant*sqrt(U)` memory.

# Speed

Performance will vary with CPU. But here are some performance numbers for an Intel i7-7567U:

```
# sums all primes up to 2^32 in 4.3s
time ./primegen $((1<<32)) -s

# prints all primes up to 2^32 in decimal in 8.4s
time ./primegen $((1<<32)) > /dev/null

# sums all primes up to 2^38 in 6m50s
time ./primegen $((1<<38)) -s

# prints all primes up to 2^38 in decimal in 10m35
time ./primegen $((1<<38)) > /dev/null
```
