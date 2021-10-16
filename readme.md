# show
slideshow thing and stuff

## features
- markdown source to slideshow rendering
- live text editing to render
- shared library code refresh

## building
cmake --build cmake-build-debug --target slider install

## developing
inotify will poll the files that you're editing and reload the libslider.so/show.md
- hack on show.md, see debug below if it's broken
- `cmake --build cmake-build-debug --target slider install`

# commands
. [*]             start of command
. font            [float] [family?] [style?] [alignment?]
    (style)       normal italic bold
    (family)      sans serif mono script
    (alignment)   left center right
. define-image    [unique-alias] [path]
    path          a file name in `/res/` dir without extension
                  , ie "lambo.png" would be `lambo`
                  , subtle note, two images must not have the same substring match (strstr)
. image           [alias-recall]
    alias-recall  recall the alias created in `define-image`
    todo          x, y, w, h
. #               [unique-alias]
                  creates a template slide (font, color, y, [Q: does it do line-height?])
. using           [alias-recall]
    alias-recall  recall the alias created in `#`
. define-style    [unique-alias]
. save-style
. style           [alias-recall]
    alias-recall  recall the alias created in `define-style`
. y               [float_h]
    float_h       a percentage of y screen real estate, .5 = 50%
. bg              [float_r] [float_g] [float_b] [? float_alpha]
. color           [float_r] [float_g] [float_b]
    float_r       text; red percentage
. line-height     [float]

# stretchy buffers
stretchy_buffer.h: a tidy little C implementation
of stretchy buffers / dynamic arrays / aka C++
vectors - declare an empty buffer with something
like 'mytype *myarray = NULL', and then use the
sb_*() functions to manipulate; read/write
individual elements by indexing as usual

# debug inotify events
inspect the show markdown file's events on your
operating system and IDE environment by executing
the inotify executable built with cmake in this
project. it is the same as the man page example,
and will print each type of event received from
the file descriptor that is being watched.

    show $ ./cmake-build-debug/inotify shows/show.md
    Press ENTER key to terminate.
    Listening for events.
    IN_OPEN: shows/show.md/ [file]
    IN_CLOSE_NOWRITE: shows/show.md/ [file]
    IN_OPEN: shows/show.md/ [file]
    IN_CLOSE_WRITE: shows/show.md/ [file]
    IN_OPEN: shows/show.md/ [file]

then take the event and modify the ino.h call to
inotify_add_watch() to make sure you're watching
the right events.

## logging
there's a bunch of macros in show.h to help with logging
https://www.cplusplus.com/reference/cstdio/printf/

    error(), info(), debug() : contain a line ending
    err(),   id(),   dbg()   : do not contain a line ending

Specifier    Output                                                                         Example
d or i       Signed decimal integer                                                         392
u            Unsigned decimal integer                                                       7235
o            Unsigned octal                                                                 610
x            Unsigned hexadecimal integer                                                   7fa
X            Unsigned hexadecimal integer (uppercase)                                       7FA
f            Decimal floating point, lowercase                                              392.65
F            Decimal floating point, uppercase                                              392.65
e            Scientific notation (mantissa/exponent), lowercase                             3.9265e+2
E            Scientific notation (mantissa/exponent), uppercase                             3.9265E+2
g            Use the shortest representation: %e or %f                                      392.65
G            Use the shortest representation: %E or %F                                      392.65
a            Hexadecimal floating point, lowercase                                          -0xc.90fep-2
A            Hexadecimal floating point, uppercase                                          -0XC.90FEP-2
c            Character                                                                      a
s            String of characters                                                           example
p            Pointer address                                                                b8000000
n            Nothing printed. The corresponding argument must be a pointer to a signed int.
             The number of characters written so far is stored in the pointed location.
%            A % followed by another % character will write a single % to the stream.       %
