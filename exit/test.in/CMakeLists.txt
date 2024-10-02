# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MyCProject C)

# Specify the C standard (optional)
set(CMAKE_C_STANDARD 99)

# Add the executable, specifying the source file
add_executable(t t.c)
add_executable(t2 t2.c)
