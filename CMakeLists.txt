cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 17)

project(chipollino)

# Add sub directories
add_subdirectory(InfInt)
add_subdirectory(Fraction)
add_subdirectory(AutomatonToImage)
add_subdirectory(Objects)
add_subdirectory(Tester)
add_subdirectory(Interpreter)
add_subdirectory(InputGenerator)
add_subdirectory(Tests)