# do it
define and use font style
soft-wraps
parse textfile to slides
hot load slideshow file
font color
change font
change font size
change font attributes mid-line
templates (including a slide in another slide) "shared content"
presenter mode
overview mode
transitions
slide editor in game
drawing on the slides
big cursor? audience viewing conditions use case
videos
hyperlinks
drag and drop show.md files
rotated text
tables

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

