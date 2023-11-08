# Set the project name
project(UnitTestsApp)

# Create a sources variable with a link to all cpp files to compile

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SOURCES
        src/Example.cpp
        src/main.cpp
        src/UnitTests.cpp)


# Add a library with the above sources
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        )

target_link_libraries(${PROJECT_NAME}
        InputGenerator
        Interpreter
        Tester
        Objects
        gtest
        )

enable_testing()
add_test(NAME UnitTests
        COMMAND ${PROJECT_NAME})