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

# Memory usage

`primegen` generate primes up to a given upper-bound U to which it will sieve.
It will use approximately U/16 bytes of RAM.

# Speed

Performance will vary with CPU. But here are some performance numbers for an Intel i7-7567U:

```
# sums all primes up to 2^32 in 4.3s
time ./primegen $((1<<32)) -s

# prints all primes up to 2^32 in 8.4s
time ./primegen $((1<<32)) > /dev/null

```
