cmake_minimum_required(VERSION 3.5)
project(pthread_racer)

find_package( Threads REQUIRED )

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread")

set(SOURCE_FILES pthread_race.c)
add_executable(pthread_racer ${SOURCE_FILES})
target_link_libraries( pthread_racer Threads::Threads )