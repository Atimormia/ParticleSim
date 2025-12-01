# ParticleSim

A high performance 2D particle simulation engine built in modern C++20.

## Current features
- Project scaffolding
- Build system
- Testing infrastructure

## Build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

## Run tests
ctest --output-on-failure