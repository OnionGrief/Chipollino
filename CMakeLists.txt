cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)

project(chipollino)

# Add sub directories
add_subdirectory(libs/Fraction)
add_subdirectory(libs/AutomatonToImage)
add_subdirectory(libs/Objects)
add_subdirectory(libs/Tester)
add_subdirectory(libs/Logger)
add_subdirectory(libs/Interpreter)
add_subdirectory(libs/InputGenerator)
add_subdirectory(apps/InterpreterApp)
add_subdirectory(apps/InputGeneratorApp)
add_subdirectory(apps/UnitTestsApp)
add_subdirectory(apps/IntegrationTestsApp)
add_subdirectory(apps/MetamorphicTestsApp)