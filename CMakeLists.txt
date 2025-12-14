cmake_minimum_required(VERSION 3.10)
project(vaulter)

# include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES "src/**/*.c")
file(GLOB SRC_FILES "src/*.c")
set(SOURCES ${SOURCES} ${SRC_FILES})

if(SOURCES)
    add_executable(vaulter ${SOURCES})
else()
    message(FATAL_ERROR "No .c files found in source")
endif()

target_link_libraries(vaulter ssl crypto)
target_compile_options(vaulter PRIVATE -Ofast)
