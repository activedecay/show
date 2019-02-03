# do it
horrible crash locks the os. holy shit. fixme now.

draw on slides with laser pointer
use voice commands to launch a missle and blow up the text

shadowed text!
all wrapped text centered on y!
- vertical-align middle around text items
background should be global without a slide
disassemble font-size with font-align

variables
- math operations
    - parenthesis

load-image lol pic.png
image scale, crop, rotate
\ escape handling
in particular, (!"needs a better font failure mechanism");
- way better error handling in general
fix memory @Leaks
- hot-load images! OMFG ? could just do the same 
  thing as reading the image each time show.md changes
rotated text
x pos
hyperlinks
drag and drop show.md files
change font attributes mid-line
presenter mode
- annotate/notes screen separate from presentation screen
- previous/next slide
- timers
- progress
- toggle between presenter and audience view during 
  a 1-screen practice
overview mode
audience viewing conditions use case
transitions
drawing on the slides
slide editor in game
videos
tables
tablets github.com/ApoorvaJ/EasyTab
slideshow progress bar
picture in a picture
game-like effects, subtle
spell-cheker

aspect ratio (hard)
image tiling (uv-scale 1 -1)
image borders
rectangles
poly lines


declare-font variables that import fonts from disk on the fly (eh)

# features
packaged with free fonts
initialize a new rect y for each y encountered. draw a box
allow all style variables to change independently
shadowed text!
many image types (png, jpg, bmp, gif)

# usage
Note: the command syntax uses `. ` as the start of a command.

Reusable slide templates allow for including a slide in another slide.
This allows a slide to share content inside other slides across the show.
Example usage: 

- declare with: `. # Company Legal Text` - declarations are hidden
- use with: `. using Company Legal Text` in another slide

Declare styles and apply them across all the text lines that follow.
Styles will apply across slide boundaries.
Example usage:
- declare a new style with: `. define-style Title Text`
- then define the style properties
    - properties of a style:
    - `. font [float size] [font-attribute]*`
    - `. margin [float]`
    - `. color [float-red] [float-green] [float-blue] [float-alpha]`
. finally save with: `. save-style`
- use with: `style trolol`

Text position flows from the most recent `. y [float-height]`.
Text position starts at the top for every new slide.

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
    
    // desktop relative
    SDL_GetGlobalMouseState(&mouse_x, &mouse_y); 

    SDL_CaptureMouse(true);
    
    window = SDL_CreateWindow("schlides!", 0,
            0, w / 2, h / 2, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1,
            SDL_RENDERER_PRESENTVSYNC);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED);

    SDL_Cursor *cursors[SDL_NUM_SYSTEM_CURSORS];
    for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i) {
        cursors[i] = SDL_CreateSystemCursor(i);
    }
    SDL_SetCursor(cursor);
    for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i) {
        SDL_FreeCursor(cursors[i]);
    }
    
    

    /*
    int free_show(slide_show *show) {
      if (!show) return 0;
    
      for (int i = 0; i < count(show->slides); ++i) {
        slide_item *slide = show->slides[i];
    
        for (int j = 0; j < count(slide->using); ++j) {
          // these are already freed with show->template_slides
          // slide_item *using_slide = slide->using[j];
          
          for (int k = 0; k < count(slide->grocery_items); ++k) {
            free(slide->grocery_items[k]->item.free_me);
            stretch_free(slide->grocery_items[k]);
          }
          for (int l = 0; l < count(slide->styles); ++l) {
            free(slide->styles[l]->name);
            slide->styles[l]->name = 0;
          }
          for (int m = 0; m < count(slide->points); ++m) {
            free(slide->points[m]);
            slide->points[m] = 0;
          }
    
          stretch_free(slide->using);
          slide->using = 0;
          stretch_free(slide->grocery_items);
          slide->grocery_items = 0;
          stretch_free(slide->styles);
          slide->styles = 0;
          stretch_free(slide->points);
          slide->points = 0;
          free(slide->title);
          slide->title = 0;
          free(slide);
          show->slides[i] = 0;
        }
      }
        
    
      template_slide *s, *tmp;
      HASH_ITER(hh, show->template_slides, s, tmp) {
        printf("slide template %s @ %p\n", s->id, s->slide);
        free(s->slide);
        s->slide = 0;
        free(s);
      }
      free(show->template_slides);
      show->template_slides = 0;
    
      style_hash *ss, *tmpp;
      HASH_ITER(hh, saved_styles, ss, tmpp) {
        printf("style hash %s @ %p\n", ss->name, ss->style);
        free(ss->style->name);
        ss->style->name = 0;
        free(ss->name);
        ss->name = 0;
        free(ss->style);
        ss->style = 0;
        free(ss);
      }
      free(saved_styles);
      saved_styles = 0;
    
      free(show);
      return 0;
    }