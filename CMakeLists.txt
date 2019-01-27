cmake_minimum_required(VERSION 3.13)
project(show C)

set(CMAKE_C_STANDARD 11)

add_library(slider SHARED src/show.c src/show.h src/lib/stretchy.h src/lib/uthash.h)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS slider DESTINATION lib)

link_directories(./lib)
add_executable(show src/main.c src/lib/stretchy.h src/lib/uthash.h)
target_link_libraries(show SDL2 SDL2_ttf dl)

add_executable(test test/test.c)