cmake_minimum_required(VERSION 3.12.3)
project(show C)

#[[ cmake is very fucking stupid for reserving the "test" keyword.
  and after asking the developers, they want me to rename the output.
  great workaround, fellas. ]]
# add_executable(test test.c) # never name an executable "test"!

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -Wno-discarded-qualifiers)

add_compile_definitions( _GNU_SOURCE=1 ) # you can now pthread_setname_np

# creates a libslider.so shared object that is installed
# in our local repository's ./lib/ directory
# our application's -L/home/jfaust/hack/show/./lib
#                   and -Wl,-rpath,/home/jfaust/hack/show/./lib
# will search for the libslider.so in ./lib/ and will watch for
# changes to the library, reload the function definitions from
# the changes. pretty sweet.
add_library(slider SHARED src/show.c src/show.h
        src/lib/stretchy.h
        src/lib/uthash.h
        src/lib/csapp.c
        )
target_link_libraries(slider)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS slider DESTINATION lib)

#link_directories(./lib) # i think this is perilous because it changes everybody's
add_executable(show src/main.c
        src/lib/stretchy.h
        src/lib/uthash.h
        src/lib/ino.h
        src/lib/csapp.c
        )
target_link_directories(show PRIVATE ./lib)
target_link_libraries(show SDL2 SDL2_ttf pthread m dl X11 OpenGL)

add_executable(image-scratch scratch/image-scratch.c)
target_link_libraries(image-scratch SDL2 m)

add_executable(inotify scratch/inotify-man-page-example.c)

add_executable(str-to-double scratch/str-to-double.c
        src/lib/csapp.c)
target_link_libraries(str-to-double pthread)

add_executable(window window.c)
target_link_libraries(window SDL2 SDL2_ttf)
