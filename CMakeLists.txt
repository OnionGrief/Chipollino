# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 17)

# Set the project name
project (chipollino)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
	src/objects/BaseObject.cpp
	src/objects/FiniteAutomaton.cpp
	src/objects/Regex.cpp
	src/objects/Tester.cpp
	src/objects/Language.cpp
	src/objects/Grammar.cpp
	src/AlphabetSymbol.cpp
	src/AutomatonToImage.cpp
	src/Example.cpp
	src/Logger.cpp
    src/main.cpp
)

# Add an executable with the above sources
add_executable(chipollino ${SOURCES})

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(chipollino
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/include/objects
)