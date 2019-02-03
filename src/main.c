//
// Created by justin on Sat Jan 26 08:30:09 MST 2019
//

#pragma clang diagnostic push
/* fuck you const! fuck you! */
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

#include "show.h"

void lol(void) {
  error("lol you ran out of memory!");
}

#define STRETCHY_BUFFER_OUT_OF_MEMORY lol;

#include "lib/stretchy.h"
#include "lib/ino.h"
#include "lib/csapp.h"

bool do_keydown(SDL_Event *event, slide_show *show,
                uint32_t *frame_delay);

font *add_font(font **, char *, char *);

int die(char *s);


void do_window(SDL_Event *event, uint32_t *frame_delay,
               int *w, int *h, bool *in_frame);

void quit(void);

void read_slideshow_file(void *filename);

void *watch_library(void *global_state);

void *watch_slideshow_file(void *ll);

typedef struct {
    char *library_file;
    char *show_file;
    sem_t game_sem;
    sem_t show_sem;
    game_lib game_library;
    font *fonts;  /* = 0; important */
    slide_show *show;
    style_item *saved_styles;  /* = 0; important */
} game_state;

void load_game_library(void *);

int
main(int argc, char *argv[]) {
  if (argc < 2) return die("usage: path/to/show.md");
  atexit(quit);

  game_state global_state = {
      "lib/libslider.so",
      argc < 2 ? 0 : argv[1],
      // mutual exclusion specifically for game lib thread and main
      {0},
      // mutual exclusion specifically for slide file thread and main
      {0},
      0,
      0,
      0,
      0
  };
  Sem_init(&global_state.game_sem, 0, 1);
  Sem_init(&global_state.show_sem, 0, 1);

  // make sure we have a game library before forking a thread
  load_game_library(&global_state);

  pthread_t tidp;
  Pthread_create(&tidp, 0, watch_slideshow_file, &global_state);
  Pthread_create(&tidp, 0, watch_library, &global_state);

  SDL_Window *window = 0;
  SDL_Renderer *renderer = 0;
  TTF_Font *font = 0;
  SDL_Texture *mouse_follow_word = 0;
  SDL_Event event;
  int w = 960;
  int h = 540;

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    return die("can't init SDL");
  if (TTF_Init() < 0)
    return die("can't init fonts");
  SDL_CreateWindowAndRenderer(
      w, h, SDL_WINDOW_RESIZABLE, &window, &renderer);

  if (!(font = TTF_OpenFont("./res/FreeSans.ttf", 72)))
    return die("can't load the font");
  SDL_Rect mouse_follow_rect = {10, 10};
  mouse_follow_word = global_state.game_library.texturize_text_func(
      renderer, font, "*", (SDL_Color) {255, 255, 255, 255},
      &mouse_follow_rect, SDL_BLENDMODE_BLEND);
  TTF_CloseFont(font);

  /* note @Evict from main */
  add_font(&global_state.fonts, "sansnormal", "./res/FreeSans.ttf");
  add_font(&global_state.fonts, "sansbold", "./res/FreeSansBold.ttf");
  add_font(&global_state.fonts, "sansitalic", "./res/FreeSansOblique.ttf");
  add_font(&global_state.fonts, "serifnormal", "./res/FreeSerif.ttf");
  add_font(&global_state.fonts, "serifbold", "./res/FreeSerifBold.ttf");
  add_font(&global_state.fonts, "serifitalic", "./res/FreeSerifItalic.ttf");
  add_font(&global_state.fonts, "mononormal", "./res/FreeMono.ttf");
  add_font(&global_state.fonts, "monobold", "./res/FreeMonoBold.ttf");
  add_font(&global_state.fonts, "monoitalic", "./res/FreeMonoOblique.ttf");
  add_font(&global_state.fonts, "scriptnormal", "./res/AlexBrush-Regular.ttf");
  add_font(&global_state.fonts, "scriptitalic", "./res/AlexBrush-Regular.ttf");
  add_font(&global_state.fonts, "scriptbold", "./res/AlexBrush-Regular.ttf");

  if (!global_state.show->slides)
    error("Error: no slides! create a slide with '# Title'");

  bool in_frame = false;
  bool quit = false;
  int mouse_x, mouse_y;
  u32 frame_delay = 16;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
        frame_delay = 0;
        break;
      }
      P(&global_state.show_sem);
      switch ((SDL_EventType) event.type) {
        case SDL_WINDOWEVENT: {
          do_window(&event, &frame_delay, &w, &h, &in_frame);
          break;
        }
        case SDL_KEYDOWN: {
          quit = do_keydown(&event, global_state.show, &frame_delay);
          break;
        }
        case SDL_MOUSEWHEEL: {
          global_state.show->index += -event.wheel.y;
          break;
        }
        default:
          break;
      }
      global_state.show->index = global_state.show->index < 0 ? 0
          : min(global_state.show->index, count(global_state.show->slides) - 1);
      V(&global_state.show_sem);
    }

//    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
//    SDL_RenderClear(renderer);
//    SDL_RenderCopy(renderer, babe, 0, 0);

    P(&global_state.show_sem);
    global_state.game_library.render_slide_func(
        renderer, w, h, global_state.show, global_state.fonts);
    V(&global_state.show_sem);

    SDL_GetMouseState(&mouse_x, &mouse_y);
    mouse_follow_rect.x = mouse_x + 20;
    mouse_follow_rect.y = mouse_y + 20;

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    if (in_frame)
      SDL_RenderCopy(renderer, mouse_follow_word,
                     0, &mouse_follow_rect);
    SDL_RenderPresent(renderer);
    SDL_Delay(frame_delay);
  }

  SDL_DestroyTexture(mouse_follow_word);
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

bool do_keydown(SDL_Event *event, slide_show *show,
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
    default:
      break;
  }
  return quit;
}

void do_window(SDL_Event *event, uint32_t *frame_delay,
               int *w, int *h, bool *in_frame) {
  switch ((SDL_WindowEventID) (*event).window.event) {
    default:
      break;
    case SDL_WINDOWEVENT_CLOSE: {
      (*frame_delay) = 0;
      break;
    }
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
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

  if (!(lib->it = dlopen("libslider.so", RTLD_LAZY))) {
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

  read_slideshow_file(global_state);
}

void read_slideshow_file(void *ll) {
  game_state *state = (game_state *) ll;

  static size_t read_len = 1024 * 4;

  if (state->show_file) {
    FILE *f = fopen(state->show_file, "r");
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
            .free_show_func(state->show, &state->saved_styles) != 0) {
      error("something terrible happened "
            "while freeing the show!");
    }

    if ((state->show = state->game_library
        .init_slides_func(idx, &state->saved_styles, content)) == 0) {
      die("this show file sucks!");
      exit(EXIT_FAILURE);
    }
    stretch_free(content);
    V(&state->show_sem);
  }
}

#pragma clang diagnostic pop
