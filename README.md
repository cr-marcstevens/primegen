# primegen

[![C++ CI](https://github.com/cr-marcstevens/primegen/actions/workflows/cpp.yml/badge.svg)](https://github.com/cr-marcstevens/primegen/actions/workflows/cpp.yml)

- `primegen.hpp`: C++ header-only library to generate small primes using Sieve of Eratosthenes
- `primegen.cpp`: Command line utility

# Getting started

```
git clone https://github.com/cr-marcstevens/primegen
cd primegen
make
./primegen 512     # print primes <= 512
./primegen 256 512 # print primes >= 256, <= 512
```

# Memory usage

`primegen` needs an upper-bound U to which it will sieve.
It will use approximately U/16 bytes of RAM.
