# priority fixes

- show cursor in show.markdown file (see mouse_follow_word)
    - pointer config (mouse image, offset, font, color, text, shadow, shown/hidden?)
    - keystroke toggles visibility
    - per-slide?
- margin; positive and negative x and y offsets; instead of having to use spaces to pad text
- slideshow progress bar
- videos
- rectangles
    - variables
- poly lines
    - variables
- background color is not saved in a template
    - modifying a template's background color changes the background color of all slides
    - recalling a template does not recall the background color saved on the template
    - todo: check that the background color is saved in the template slide
    - todo: when recalling a template, set the background color to the template's background color
    - todo: consider whether the current background color should be saved in a global variable
    - todo: background should be global without a slide

# killer features

- drag and drop show.markdown files to swap between which file to show
- live captions from voice
- voice commands for next slide
- draw on slides with laser pointer; follow mouse with laser pointer;
    - raspberrypi camera pointed at the slideshow, client/server sending remote commands to our slideshow program
- actually use the gpu to render; consider why the fps performance is lackluster
- y is not saved in font style; should it be? it is saved in templates
- text kerning
- frame rate limit; use case to allow the fps to be tunable
- consider moving on_keydown, on_window, and on_mouse into a shared library function call
- auto-pilot; advances slides through to the end and quits the show (a good qa test driver)
- text variables; store some text in a variable and recall it without having to use a template (why?)
- new cursor text settings per slide
- font family definition in show.markdown; `. declare-font` imports new, user-defined fonts
- split out the default family declarations `. with default-fonts`, etc.; i.e., a `with' command seems useful
- use show.markdown file as window size config: fullscreen, x, y, w, h
- text vertical align; use case 2(^hard)-like (sub)superscripts
- text margins_x (DEFAULT_STYLE is 0.05f) only used in right/left justify
- move the initialization of font families out of main? @Evict
- stretchy was deprecated, consider std_ds
- consider -fno-strict-aliasing https://blog.regehr.org/archives/1307
- consider using sscanf instead of strtok_r
- break apart amdgpu valgrind errors by extracting showlib sans sdl
    - and consider doing a cli-only slideshow implementation
    - goal: to narrow down memory leaks in our code
    - unless, we can just tune valgrind output to ignore non-show.c-code errors
    - suppression files are hard to get the hang of,
      and sdl errors all over even when we think we've been good
- image rotate
- see the code assert (!"needs a better font failure mechanism");
    - way better error handling in general
- fix memory @Leaks in valgrind
    - vertical-align middle around text items
- decouple font command that requires font-size before font(align, family, style)
- math operations; would be nice to let the slide show do the math for you.
- math language rendering for formulas
- \ escape handling; which would allow . at the front, but just ' .' works (meh)
- rotated text
- hyperlinks; image hyperlinks
- change font attributes mid-line
- presenter mode:
    - annotate/notes screen separate from presentation screen
    - previous/next slide
    - timers
    - progress
    - toggle between presenter and audience view during a 1-screen practice
- overview mode - renders all slides on one page; renders 4 slides in quadrants;
- audience viewing conditions (what are the use cases for a single show presented to multiple venues?)
- transitions
- drawing on the slides
- slide editor in game
- tables
- tablets github.com/ApoorvaJ/EasyTab
- picture in a picture
- game-like effects, subtle
- aspect ratio (hard)
- image tiling (uv-scale 1 -1)
- image borders

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
