# Set the project name
project(InfInt)

# Add a library with the above sources
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
        INTERFACE ${PROJECT_SOURCE_DIR}/include
        )