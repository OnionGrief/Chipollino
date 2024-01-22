# Set the project name
project(Interpreter)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
        src/Interpreter.cpp
        src/Interpreter.Lexer.cpp
        src/TemplateGenerator.cpp)

# Add a library with the above sources
add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        )

target_link_libraries(${PROJECT_NAME}
        Tester
        InputGenerator
        Objects
        Logger
        AutomataParser
        FuncLib
        )