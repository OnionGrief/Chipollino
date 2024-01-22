# Set the project name
project(AutomataParser)

# Lexy implementing
include(FetchContent)
FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
FetchContent_MakeAvailable(lexy)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
        src/Lexer.cpp
        src/Parser.cpp)

# Add a library with the above sources
# add_library(${PROJECT_NAME} ${SOURCES})

# Executable for testing TODO: delete
add_executable(${PROJECT_NAME} src/Parser.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE foonathan::lexy
    Objects
)
        