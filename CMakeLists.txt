cmake_minimum_required(VERSION 3.12.3)
project(show C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -Wno-discarded-qualifiers)

add_library(slider SHARED src/show.c src/show.h
        src/lib/stretchy.h
        src/lib/uthash.h
        src/lib/csapp.c
        )
target_link_libraries(slider)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS slider DESTINATION lib)

link_directories(./lib)
add_executable(show src/main.c
        src/lib/stretchy.h
        src/lib/uthash.h
        src/lib/ino.h
        src/lib/csapp.c
        )
target_link_libraries(show SDL2 SDL2_ttf pthread m dl)

#[[ cmake is retarded. why the fuck did you reserve the "test" keyword?
 jesus christ. and the developers want me to rename my output. so dumb. ]]
add_executable(test_ test/test.c)
target_link_libraries(test_ SDL2 m)
