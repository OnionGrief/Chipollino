# Set the project name
project (InterpreterApp)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
	src/main.cpp)

# Add a library with the above sources
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
	Interpreter
)