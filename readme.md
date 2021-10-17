# show
slideshow thing and stuff
requirements: a show.markdown file
              ./lib/libslider.so

## features
- markdown source to slideshow rendering
- live text editing to render
- shared library code refresh

## building
    cmake --build cmake-build-debug --target all install

## developing
inotify will poll the files that you're editing and reload the libslider.so/show.md
- hack on show.md, see debug below if it's broken
- `cmake --build cmake-build-debug --target slider install`

# commands
. [*]             the dot-space sequence '. ' is start of command
. font            [float_size] [family?] [style?] [alignment?]
    size          1/100th of a window height, `.2` = font size 20% window height
    (family)      sans serif mono script
    (style)       normal italic bold
    (alignment)   left center right
. define-image    [unique-alias] [path]
    alias         used in the `image` command to recall an image
    path          a file name substring existing in `/res/` dir
                  , ie "lambo.png" would be `lambo`
                  , subtle note, two images must not have the same substring match (strstr)
. image           [alias-recall]
    alias-recall  recall the alias created in `define-image` [Q: does image templating work?]
    todo          x, y, w, h
. #               [unique-alias]
    alias         creates a template slide (font, color, y, [Q: does it do line-height?])
. using           [alias-recall]
    alias-recall  recall the alias created in `#` [Q: what is recalled?] [Q: what happens to the text?]
. define-style    [unique-alias]
. save-style
. style           [alias-recall]
    alias-recall  recall the alias created in `define-style`
. y               [float_h]
    float_h       a percentage of y screen real estate, .5 = 50%
. bg              [float_r] [float_g] [float_b] [? float_alpha]
    float r g b   a percentage of color; .5 = 50% red, etc.
    float_alpha   slide transition fade with values < 1 (default 1)
. color           [float_r] [float_g] [float_b]
    float r g b   text; .5 = 50% red, etc.
. line-height     [float]
    height        squish text together set values < 1; (default 1)

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
