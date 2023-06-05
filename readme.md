# features
- show.markdown source file is rendered in a slideshow window
- live text editing to render changes when the show.markdown file is saved
- template slides with images and text (watermark, logo, etc.)
- 4 fonts (serif, sans-serif, monospace, cursive script)
    - each with 3 styles (normal, italic, bold)
- text color with alpha
- slide background color (with alpha for fade transitions)
- text shadow and shadow color
- hot-load images (new images can be inserted while the show is running)
- image types supported: png, jpg, bmp, gif
- image scale, crop
## dev features
- shared library code refresh allows running programs to have their implementation changed

# show
program requirements:
1. render a show.markdown file (a text file that is reloaded when changed while running)
2. ./lib/libslider.so (a dynamic library that is reloaded while running)
3. ./res/ images and fonts (a directory of files that are required to be present to run)
   - fonts are loaded from the ./res/ directory, and are not hot-loaded (todo)
   - images are hot-loaded from the ./res/ directory

# building
    cmake --build cmake-build-debug --target all install

# developing
inotify will poll the files that you're editing and reload the libslider.so and show.markdown files
- hack on show.md, see debug below if it's broken
- `cmake --build cmake-build-debug --target slider install`

# keyboard shortcuts
                   q, esc: quit
                     home: go to first slide
                      end: go to last slide
         pageup, left, up: go to previous slide
    pagedown, right, down: go to next slide
                        x: unlimited frame count

# usage in show.markdown
- markdown titles (using the '#' character) start new slides (this text is ignored).
- regular paragraphs in the markdown file appears on the slides.
- text position flows from the most recent `. y [pos]` vertical position command. (todo consider implementing margins).
- text does not wrap; you're in charge of newlines, but inserting newlines automatically inserts text vertically.
- text y position starts at the top for every new slide encountered.
- font size is 1/100th of a window height, so .5 = 50% of screen size font height (that's big text, btw).
- commands start with '. ' string on a new line.
- bulleted lists can be written, e.g., ' .', '*' or '- ' which is typical for Markdown documents.
- background, text, and text shadow colors are editable; a background alpha < 1 will fade in when transistioning slides.
- images are referenced by basename without the extension in the res directory; e.g., lambo.png is referenced as 'lambo'.

# commands documentation
The first column is "Templ", which indicates whether the command is remembered in a template during template recall.

| Templ | Text or command | Command arguments  | Explanation                                                                           |
|-------|-----------------|--------------------|---------------------------------------------------------------------------------------|
|       | ; [*]           |                    | comment                                                                               |
|       | [*]             |                    | any text that is not a command start is shown on a slide. markdown paragraph text.    |
|       |                 |                    | until margins are implemented, use spaces to insert space to the left.                |
|       | . [*]           |                    | the dot-space sequence '. ' is start of command                                       |
| yes   | . bg            |                    | [float_r] [float_g] [float_b] [? float_alpha=1]                                       |
|       |                 | float r g b        | a percentage of color; .5 = 50% red, etc.                                             |
|       |                 | float_alpha        | slide transition fade with values < 1 (default 1)                                     |
|       |                 |                    | note: using bg on a template will modify every bg from then on (this is a bug)        | 
|       | . define-image  |                    | [unique-alias] [path]                                                                 |
|       |                 | alias              | used in the `image` command to recall an image                                        |
|       |                 | path               | a file name substring existing in `/res/` dir                                         |
|       |                 |                    | , ie "lambo.png" would be `lambo`                                                     |
|       |                 |                    | , subtle note, two images must not have the same substring match (strstr)             |
| yes   | . image         |                    | [alias-recall] [? texture_x] [? y] [? w] [? h] [? screen_x] [? y] [? w] [? h]         |
|       |                 | alias              | recall the alias created in `define-image`. defaults to whole image stretched to fill |
|       |                 | (texture x y w h)  | float values for starting/ending texture coordinates (crops textures)                 |
|       |                 | (screen  x y w h)  | float values for starting/ending screen coordinates (scales textures)                 |
|       | . #             |                    | [unique-alias]                                                                        |
|       |                 | alias              | creates a template slide alias to package: images, text, font, color, y, line-height  |
|       | . using         |                    | [alias-recall]                                                                        |
|       |                 | alias              | recall the alias created in `#` [Q: what is recalled?] [Q: what happens to the text?] |
|       | . define-style  |                    | [unique-alias]                                                                        |
|       |                 | alias              | creates a style to set [Q: doc what style settings are saved (A: not .y)]             |
|       | . save-style    |                    | takes no arguments and saves the style state changes                                  |                
|       | . style         |                    | [alias-recall]                                                                        |
|       |                 | alias              | recall the alias created in `define-style`                                            |
| no    | . y             |                    | [float_h]                                                                             |
|       |                 | float_h            | a percentage of y screen real estate, .5 = 50%                                        |
| yes   | . margin        |                    | [float_horizontal] [?float_vertical]                                                  |
|       |                 | float_horizontal   | a percentage of x screen real estate, .5 = 50%                                        |                                        
|       |                 | (float_vertical)   | a percentage of y screen real estate, .5 = 50% (todo implement)                       |
| yes   | . font          |                    | [float_size] [family?] [style?] [alignment?]                                          |
|       |                 | size               | 1/100th of a window height, `.2` = font size 20% window height                        |
|       |                 | (family)           | sans serif mono script                                                                |
|       |                 | (style)            | normal italic bold                                                                    |
|       |                 | (alignment)        | left center right                                                                     |
| yes   | . color         |                    | [float_r] [float_g] [float_b] [? float_alpha=1]                                       |
|       |                 | float r g b (a)    | text; .5 = 50% red, etc.                                                              |
|       | . shadow        |                    | [float_r] [float_g] [float_b] [? float_alpha=1]                                       |
|       |                 | float r g b (a)    | text; .5 = 50% red, etc.                                                              |
| yes   | . line-height   |                    | [float]                                                                               |
|       |                 | height             | squish text together set values < 1; (default 1)                                      |

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

# glxinfo
interesting things; install mesa-utils

    diff <(glxinfo) <(glxinfo -l)

glx - this is the x window system extensions lib for gl


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
