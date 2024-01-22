# Set the project name
project(FuncLib)

# Add a library with the above sources
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
        INTERFACE ${PROJECT_SOURCE_DIR}/include
        )

target_link_libraries(${PROJECT_NAME} INTERFACE
        Objects
        )