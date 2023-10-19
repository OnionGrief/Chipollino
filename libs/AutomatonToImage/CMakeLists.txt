# Set the project name
project(AutomatonToImage)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
        src/AutomatonToImage.cpp)

# Add a library with the above sources
add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        )