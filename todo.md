# do it

## killer features
- live captions from voice
- voice commands for next slide
- draw on slides with laser pointer; follow mouse with laser pointer;
  - raspberrypi camera pointed at the slideshow, client/server sending remote commands to our slideshow program

consider rendering templates and images in the proper order because background images overwrite template text
y is not saved in font style; should it be?
consider how images "work" on templates, if at all
actually use the gpu to render; consider why the fps performance is lackluster
frame rate limit; use case to allow the fps to be tunable
consider moving on_keydown, on_window, and on_mouse into a shared library function call
auto-pilot; advances slides through to the end and quits the show (a good qa test driver)
text variables; store some text in a variable and recall it without having to use a template
show cursor in show.markdown settings; keystroke toggle
new cursor text settings per slide
font family definition in show.markdown; `. declare-font` imports new, user-defined fonts
  - split out the default family declarations `. with default-fonts`, etc.; i.e., a `with' command seems useful
drag and drop show.markdown files to swap between which file to show
text +/- x offset; use case instead of having to use spaces to pad text
use show.markdown file as pointer config: show mouse, offset-(x,y), word, font, color (see mouse_follow_word)
use show.markdown file as window size config: fullscreen, x, y, w, h
text vertical align; use case 2(^hard)-like (sub)superscripts
text margins_x (DEFAULT_STYLE is 0.05f) only used in right/left justify
move the initialization of font families out of main? @Evict
stretchy was deprecated, consider std_ds
consider -fno-strict-aliasing https://blog.regehr.org/archives/1307
consider using sscanf instead of strtok_r
break apart amdgpu valgrind errors by extracting showlib sans sdl
  - and consider doing a cli-only slideshow implementation 
  - goal: to narrow down memory leaks in our code
  - unless, we can just tune valgrind output to ignore non-show.c-code errors
  - suppression files are hard to get the hang of,
    and sdl errors all over even when we think we've been good
image scale, crop, rotate
see the code assert (!"needs a better font failure mechanism");
- way better error handling in general
fix memory @Leaks in valgrind
- vertical-align middle around text items
background should be global without a slide
decouple font command that requires font-size before font(align, family, style)
math operations; would be nice to let the slide show do the math for you.
math language rendering for formulas
\ escape handling; which would allow . at the front, but just ' .' works (meh)
rotated text
hyperlinks; image hyperlinks
change font attributes mid-line
presenter mode
- annotate/notes screen separate from presentation screen
- previous/next slide
- timers
- progress
- toggle between presenter and audience view during
  a 1-screen practice
overview mode - renders all slides on one page; renders 4 slides in quadrants;
audience viewing conditions (what are the use cases for a single show presented to multiple venues?)
transitions
drawing on the slides
slide editor in game
videos
tables
tablets github.com/ApoorvaJ/EasyTab
slideshow progress bar
picture in a picture
game-like effects, subtle
aspect ratio (hard)
image tiling (uv-scale 1 -1)
image borders
rectangles
poly lines

# features
hot-load images; meaning we find new images every time a new one is `. define-image`d
normal, italic, and bold versions of script text
remove threaded manipulation of textures (allowed on the main thread only!)
text shadow color
define-image lol pic.png
packaged with free fonts
initialize a new rect y for each y encountered. draw a box
allow all style variables to change independently
many image types (png, jpg, bmp, gif)
shadowed text!

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