cmake_minimum_required(VERSION 3.5)
project(pandc)

find_package( Threads REQUIRED )

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread")

set(SOURCE_FILES pandc.c)
add_executable(pandc ${SOURCE_FILES})
target_link_libraries( pandc Threads::Threads )