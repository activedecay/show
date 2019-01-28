# do it
segfaults on load library
when mouse leaves, hide mouse follower text
font color
style variables
stateful soft-wraps (bug in resize where y-computed weirdly)
images
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
slide editor in game
videos
rotated text
tables
tablets github.com/ApoorvaJ/EasyTab

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

