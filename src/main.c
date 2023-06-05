//
// Created by justin on Sat Jan 26 08:30:09 MST 2019
// code review Sat Oct 16 09:33:24 MDT 2021
//

#pragma clang diagnostic push
/* consider discarding const qualifiers */
#pragma clang diagnostic ignored \
  "-Wincompatible-pointer-types-discards-qualifiers"
#pragma ide diagnostic ignored "cert-msc30-c" // rand is weak
#pragma ide diagnostic ignored "cert-msc32-c" // rand is weak
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCDFAInspection"

#include <dirent.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <wait.h>
#include <fcntl.h>
#include <X11/X.h> // can probably remove -lX11 from cmake, too
#include <X11/Xlib.h> // can probably remove -lX11 from cmake, too
#include <GL/gl.h>

#include "show.h"

void stretchy_oom(void) {
  error("stretchy buffers ran out of memory!");
}

#define STRETCHY_BUFFER_OUT_OF_MEMORY stretchy_oom;

#include "lib/stretchy.h"
#include "lib/ino.h"
#include "lib/csapp.h"

bool on_keydown(SDL_Event *event, slide_show *show, uint32_t *frame_delay);

font *add_font(font **, char *, char *);

int die(char *);

void on_window(SDL_Event *event, uint32_t *frame_delay,
               int *w, int *h, bool *in_frame);

void quit(void);

void read_slideshow_file(void *);

void *watch_library(void *);

void *watch_slideshow_file(void *);

typedef struct {
    char *library_file; /* string path to the library */
    char *show_file; /* string path to the show.markdown file */
    sem_t game_sem; /* the library mutex to protect the game_lib reassignments */
    sem_t show_sem;
    game_lib game_library;
    font *fonts;  /* = 0; important */
    slide_show *show;
    style_item *saved_styles;  /* = 0; important */
    SDL_Renderer *renderer;
    linkedlist *images;
} game_state;

void load_game_library(void *);

void sdl_hack_logger(void *userdata, int category, SDL_LogPriority priority, const char *message) {
  // a nice place to set breakpoints
  // but it leads to more debugging questions
  // about how to set up the xseterrorhandler
  fprintf(stderr, "lmao"); // set breakpoints here to make your live easier.
}

/*
int x_error_handler(Display *a, XErrorEvent *b) {
  error("wtf though"); // this doesn't even work. can't break here.
  return 0;
}
*/

#define errExitErrno(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); \
  } while (0)

int
main(int argc, char *argv[]) {
  if (argc < 2) return die("usage: path/to/show-markdown");

  atexit(quit);
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

#if (sillyopenglwindow)
  {
    SDL_Window *window;
    SDL_GLContext context;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // 24 bits
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // enable

    window = SDL_CreateWindow("OpenGL Window", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
    if (!window) die(SDL_GetError());

    context = SDL_GL_CreateContext(window);
    if (!context) die(SDL_GetError());

    int r, g, b;
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);

    printf("Red size: %d, Green size: %d, Blue size: %d\n", r, g, b);
    glClearColor(1.f,1.f,1.f,1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window);

    SDL_Delay(5000);
  }
#endif

  /* more epic logging stuff note: works well with SetOutputFunction below */
  // SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
  /* epic logging thing to help solve issues! */
  //SDL_LogSetOutputFunction(sdl_hack_logger, 0);
  // todo figure out how this is supposed to work
  // because i can't break in the x_error_handler
  // XSetErrorHandler(x_error_handler);

  linkedlist images = {0};
  game_state game_state = {
    // note, you need -L./lib when compiling to make this work
    "lib/libslider.so",
    argc < 2 ? 0 : argv[1],
    {0},
    {0},
    {
      0,
      0,
      0,
      0,
      0,
      0,},
    0,
    0,
    0,
    0,
    &images,
  };
  Sem_init(&game_state.game_sem, 0, 1);
  Sem_init(&game_state.show_sem, 0, 1);

  SDL_Window *window = 0;
  SDL_Renderer *renderer = 0;
  SDL_Event event;
  int w = 960;
  int h = 540;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    return die("can't init SDL");
  if (TTF_Init() < 0)
    return die("can't init SDL TTF");
  SDL_CreateWindowAndRenderer(
    w, h, SDL_WINDOW_RESIZABLE, &window, &renderer);

  game_state.renderer = renderer;

  // make sure we have a game library before forking a thread
  load_game_library(&game_state);
  read_slideshow_file(&game_state);

  if (!game_state.show->slides)
    return die("Error: no slides! create a slide with '# Title'");

  pthread_t threadId;
  Pthread_create(&threadId, 0, watch_slideshow_file, &game_state);
#define pthread_name_len 16
  char thread_name[pthread_name_len] = "show watch";
  Pthread_setname_np(threadId, thread_name);
  Pthread_create(&threadId, 0, watch_library, &game_state);
  strncpy(thread_name, "lib watch", pthread_name_len);
  Pthread_setname_np(threadId, thread_name);

  /* note @Evict from main, i dunno make because we wanted to make a font-creation fun? */
  add_font(&game_state.fonts, "sansnormal", "./res/FreeSans.ttf");
  add_font(&game_state.fonts, "sansbold", "./res/FreeSansBold.ttf");
  add_font(&game_state.fonts, "sansitalic", "./res/FreeSansOblique.ttf");

  add_font(&game_state.fonts, "serifnormal", "./res/FreeSerif.ttf");
  add_font(&game_state.fonts, "serifbold", "./res/FreeSerifBold.ttf");
  add_font(&game_state.fonts, "serifitalic", "./res/FreeSerifItalic.ttf");

  add_font(&game_state.fonts, "mononormal", "./res/FreeMono.ttf");
  add_font(&game_state.fonts, "monobold", "./res/FreeMonoBold.ttf");
  add_font(&game_state.fonts, "monoitalic", "./res/FreeMonoOblique.ttf");

  add_font(&game_state.fonts, "scriptnormal", "./res/Quintessential-Regular.ttf");
  add_font(&game_state.fonts, "scriptitalic", "./res/AlexBrush-Regular.ttf");
  add_font(&game_state.fonts, "scriptbold", "./res/OleoScript-Bold.ttf");

  bool in_frame = false;
  bool quit = false;
  u32 frame_delay = 16;

  SDL_ShowCursor(false);
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      const Uint8 *state = SDL_GetKeyboardState(NULL);
      if (state[SDL_SCANCODE_RETURN]) {
        printf("<RETURN> is pressed.\n");
      }
      if (state[SDL_SCANCODE_RIGHT] && state[SDL_SCANCODE_UP]) {
        printf("Right and Up Keys Pressed.\n");
      }

      if (event.type == SDL_QUIT) {
        quit = true;
        frame_delay = 0;
        break;
      }
      P(&game_state.show_sem);
      switch ((SDL_EventType) event.type) {
        case SDL_WINDOWEVENT: {
          on_window(&event, &frame_delay, &w, &h, &in_frame);
          break;
        }
        case SDL_KEYDOWN: {
          quit = on_keydown(&event, game_state.show, &frame_delay);
          break;
        }
        case SDL_MOUSEWHEEL: {
          game_state.show->index += -event.wheel.y;
          break;
        }
        default:
          break;
      }
      game_state.show->index =
        game_state.show->index < 0 ? 0
                                   : min(game_state.show->index,
                                         count(game_state.show->slides) - 1);
      V(&game_state.show_sem);
    }

    P(&game_state.show_sem);
    if (game_state.show->slides)
      game_state.game_library.render_slide_func(
        renderer, w, h, game_state.show, game_state.fonts, game_state.images, in_frame);
    V(&game_state.show_sem);

    SDL_RenderPresent(renderer);
    SDL_Delay(frame_delay);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  return EXIT_SUCCESS;
}

void quit(void) {
  SDL_Quit();
  TTF_Quit();
}

font *add_font(font **fonts, char *name, char *filepath) {
  font *found;
  HASH_FIND_STR(*fonts, name, found);
  if (found) return found;
  found = Malloc(sizeof(font));
  found->id = name;
  found->filename = filepath;
  HASH_ADD_STR(*fonts, id, found);
}

bool on_keydown(SDL_Event *event, slide_show *show,
                uint32_t *frame_delay) {
  bool quit = false;

  switch ((SDL_Scancode) (*event).key.keysym.scancode) {
    case SDL_SCANCODE_Q:
      quit = true;
      (*frame_delay) = 1;
      break;
    case SDL_SCANCODE_ESCAPE:
      quit = true;
      (*frame_delay) = 1;
      break;
    case SDL_SCANCODE_HOME:
      show->index = 0;
      break;
    case SDL_SCANCODE_PAGEUP:
      show->index--;
      break;
    case SDL_SCANCODE_END:
      show->index = count(show->slides) - 1;
      break;
    case SDL_SCANCODE_PAGEDOWN:
      show->index++;
      break;
    case SDL_SCANCODE_RIGHT:
      show->index++;
      break;
    case SDL_SCANCODE_LEFT:
      show->index--;
      break;
    case SDL_SCANCODE_DOWN:
      show->index++;
      break;
    case SDL_SCANCODE_UP:
      show->index--;
      break;
    case SDL_SCANCODE_X:
      (*frame_delay) = 0;
      break;
    default:
      break;
  }
  /* keyboard shortcuts:
   *                q, esc: quit
   *                  home: go to first slide
   *                   end: go to last slide
   *      pageup, left, up: go to previous slide
   * pagedown, right, down: go to next slide
   *                     x: unlimited frame rate
   */
  return quit;
}

void on_window(SDL_Event *event, uint32_t *frame_delay,
               int *w, int *h, bool *in_frame) {
  switch ((SDL_WindowEventID) (*event).window.event) {
    default:
      break;
    case SDL_WINDOWEVENT_CLOSE: {
      (*frame_delay) = 0;
      break;
    }
    case SDL_WINDOWEVENT_SIZE_CHANGED: { // aka resize resizable
      *w = event->window.data1;
      *h = event->window.data2;
      break;
    }
    case SDL_WINDOWEVENT_ENTER:
    case SDL_WINDOWEVENT_FOCUS_GAINED: {
      *in_frame = true;
      (*frame_delay) = 16;
      break;
    }
    case SDL_WINDOWEVENT_LEAVE:
    case SDL_WINDOWEVENT_FOCUS_LOST: {
      *in_frame = false;
      (*frame_delay) = 150;
      break;
    }
  }
}

int die(char *s) {
  error("sorry, charlie: %s", s);
  return EXIT_FAILURE;
}

/* child thread routine */
void *watch_slideshow_file(void *global_state) {
  game_state *state = (game_state *) global_state;
  inotify(state->show_file, read_slideshow_file, global_state);
}

/* child thread routine */
void *watch_library(void *global_state) {
  game_state *state = (game_state *) global_state;
  inotify(state->library_file, load_game_library, global_state);
}

void load_game_library(void *global_state) {
  game_state *state = (game_state *) global_state;
  P(&state->game_sem);
  game_lib *lib = &state->game_library;
  if (lib->it) dlclose(lib->it);

  if (!(lib->it = dlopen(state->library_file, RTLD_LAZY))) {
    error("dlopen failed %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  dlerror(); // clear existing errors

  lib->init_slides_func = dlsym(lib->it, "init_slides");
  lib->render_slide_func = dlsym(lib->it, "render_slide");
  lib->texturize_text_func = dlsym(lib->it, "texturize_text");
  lib->free_show_func = dlsym(lib->it, "free_show");
  info("loaded new library! %p", lib->it);
  V(&state->game_sem);

  char *error;
  if ((error = dlerror()) != 0) {
    fprintf(stderr, "todo lasagna %s\n", error);
    exit(EXIT_FAILURE);
  }
}

void read_slideshow_file(void *ll) {
  game_state *state = (game_state *) ll;

  static size_t read_len = 1024 * 4;

  if (state->show_file) {
    FILE *f = Fopen(state->show_file, "r");
    char *content = 0;
    size_t total = 0;
    char *next;
    bool done = false;
    size_t rc;
    while (!done) {
      next = grow_by(content, read_len);
      total += (rc = fread(next, sizeof(char), read_len, f));
      if (rc == 0) done = true;
    }
    content[total] = 0;
    fclose(f);

    P(&state->show_sem);

    int idx = state->show ? state->show->index : -1;

    if (state->game_library
          .free_show_func(state->show, &state->saved_styles) != 0)
      error("something terrible happened "
            "while freeing the show!");

    if ((state->show = state->game_library
      .init_slides_func(idx,
                        &state->saved_styles,
                        content)) == 0) {
      die("this show file sucks!");
      exit(EXIT_FAILURE);
    }
    stretch_free(content);
    V(&state->show_sem);
  } else {
    error("how did we get here?");
  }
}

#pragma clang diagnostic pop
