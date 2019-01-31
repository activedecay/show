# do it
declare-font variables that import fonts from disk on the fly
assert(!"needs a better font failure mechanism");
text shadow (wtf alpha on text is broken)
way better error handling
vertical-align middle and shit.
fix memory @Leaks
images
more image formats than bmp
hot-load images! OMFG ? could just do the same thing as reading the image each time show.md changes
image crop, rotate
rotated text
x pos
hyperlinks
drag and drop show.md files
change font attributes mid-line
presenter mode
overview mode
audience viewing conditions use case
transitions
drawing on the slides
templates (including a slide in another slide) "shared content"
declare-slide variables (templates)
- slide visibility set to no for templates
- use-slide command to include templates in the current slide
- can't use the same name for the templates
- use a shown-slide list
slide editor in game
videos
tables
tablets github.com/ApoorvaJ/EasyTab
slideshow progress bar
picture in a picture
game-like effects, subtle
spell-cheker

# features
declare-style variables
properties of a style:
  margin
package of free fonts
initialize a new rect y for each y encountered. draw a box
allow all style variables to change independently

# usage
font size is 1/100th of a screen height

# notes

# dead code

    switch(SDL_EventType)
        case SDL_DROPFILE:break;
        case SDL_DROPTEXT:break;
        case SDL_DROPBEGIN:break;
        case SDL_DROPCOMPLETE:break;

    switch(hmm..)
        case SDL_TEXTEDITING:break;
        case SDL_TEXTINPUT:break;

    SDL_GetRelativeMouseMode();
    good way to wrote a fps shooter
    SDL_ShowCursor(false);
    SDL_SetRelativeMouseMode(true);
    
    SDL_GetGlobalMouseState(&mouse_x, &mouse_y); // desktop relative

    SDL_CaptureMouse(true);
    
    window = SDL_CreateWindow("schlides!", 0, 0, w / 2, h / 2, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

