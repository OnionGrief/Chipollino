# Set the project name
project(Objects)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
        src/TransformationMonoid.cpp
        src/BaseObject.cpp
        src/FiniteAutomaton.cpp
        src/Regex.cpp
        src/Language.cpp
        src/Grammar.cpp
        src/AlphabetSymbol.cpp
        src/AlgExpression.cpp
        src/AbstractMachine.cpp
        src/MemoryFiniteAutomaton.cpp
        src/BackRefRegex.cpp
        )

# Add a library with the above sources
add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        )

target_link_libraries(${PROJECT_NAME}
        Fraction
        AutomatonToImage
        gtest
        )