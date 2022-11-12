# Set the project name
project (Tester)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
	src/Tester.cpp)

# Add a library with the above sources
add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
 	Objects
)