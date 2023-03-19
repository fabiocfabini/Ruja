cmake_minimum_required(VERSION 3.10)
project(Coruja C)

file(GLOB_RECURSE SOURCES "*.c" ../ruja.c)
add_executable(ruja ${SOURCES})
set_target_properties(ruja PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)

target_compile_options(ruja PRIVATE -Wall -Wextra -std=c17 -pedantic -ggdb -Wswitch-enum)

add_custom_target(dirs
    COMMAND mkdir -p ./out/log 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_custom_target(go
    COMMAND ruja input.ruja
    DEPENDS dirs ruja
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_custom_target(mem
    COMMAND valgrind --leak-check=full --show-leak-kinds=all -s --log-file=./out/log/valgrind.log ./bin/ruja input.ruja
    DEPENDS dirs ruja
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
)