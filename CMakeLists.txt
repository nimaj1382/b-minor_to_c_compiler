cmake_minimum_required(VERSION 3.23)
project(b-minor_to_c_compiler C)

set(CMAKE_C_STANDARD 11)  # C23 might not be widely supported, C11 is safer

# Add source files
set(SOURCE_FILES
        main.c
        ast.c
        lexer.c
        parser.c
        codegen.c
        )

# Add header files
set(HEADER_FILES
        ast.h
        lexer.h
        parser.h
        codegen.h
        )

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# Link with math library
target_link_libraries(${PROJECT_NAME} m)

# Enable warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Define custom command to print date and login
add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Building B-minor compiler on date: ${CMAKE_CURRENT_LIST_DIR}"
)

# Create example b-minor file in build directory for testing
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example.b ${CMAKE_CURRENT_BINARY_DIR}/example.b COPYONLY)
