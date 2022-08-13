cmake_minimum_required(VERSION 3.23)

project(stencils-openmpi)

set(CMAKE_C_STANDARD 99)

find_package(MPI)

add_executable(stencils-openmpi src/main.c src/FileReader.c src/Color.c include/Color.h src/FixedPoint.c include/FixedPoint.h)