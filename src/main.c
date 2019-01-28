#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc30-c" // rand is weak
#pragma ide diagnostic ignored "cert-msc32-c" // rand is weak
//
// Created by justin on Sat Jan 26 08:30:09 MST 2019
//

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

void quit(void);

void lol(void) {
  error("lol you ran out of memory");
}

#define STRETCHY_BUFFER_OUT_OF_MEMORY lol;

#include "lib/stretchy.h"
#include "lib/ino.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wimplicit-int"
#pragma ide diagnostic ignored "OCDFAInspection"




void read_slideshow_file(int signal);

init_slides_ptr make_slides;
render_slide_ptr draw_slide;
texturize_text_ptr make_text;
char *slide_show_file = 0;
size_t read_len = 24;
slide_show *show;
void *game_lib;

void
load_game_library(int _) {
  if (game_lib) dlclose(game_lib);

  if (!(game_lib = dlopen("libslider.so", RTLD_LAZY))) {
    fprintf(stderr, "bitch %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  dlerror(); // clear existing errors

  make_slides       = dlsym(game_lib, "init_slides");
  draw_slide        = dlsym(game_lib, "render_slide");;
  make_text         = dlsym(game_lib, "texturize_text");;
  info("shh s'alibrary %p", game_lib);

  char *error;
  if ((error = dlerror()) != 0) {
    fprintf(stderr, "todo lasagna %s\n", error);
    // exit(EXIT_FAILURE);
  }

  read_slideshow_file(SIGCONT);
}

void
do_window(SDL_Event *event, uint32_t *frame_delay, int *w, int *h) {
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
      (*frame_delay) = 16;
      break;
    }
    case SDL_WINDOWEVENT_LEAVE:
    case SDL_WINDOWEVENT_FOCUS_LOST: {
      (*frame_delay) = 200;
      break;
    }
  }
}

bool do_keydown(SDL_Event *event, slide_show *show, uint32_t *frame_delay) {
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


int
die(char *s) {
  error("sorry, charlie: %s", s);
  return EXIT_FAILURE;
}

int
main(int argc, char *argv[]) {
  load_game_library(0);
  atexit(quit);

  if (argc < 2) {
    error("usage: path/to/show.md");
  }

  slide_show_file = argc < 2 ? 0 : argv[1];

  if (fork() == 0) {
    inotify(slide_show_file, SIGUSR1); // child
    return 0;
  }
  if (fork() == 0) {
    inotify("lib/libslider.so", SIGUSR2); // child
    return 0;
  }

  signal(SIGUSR1, read_slideshow_file);
  signal(SIGUSR2, load_game_library);

  SDL_Window *window = 0;
  SDL_Renderer *renderer = 0;
  SDL_Surface *babe_surface = 0;
  SDL_Texture *babe = 0;
  TTF_Font *font = 0;
  SDL_Texture *mouse_follow_word = 0;
  SDL_Surface *cursor_surface = 0;
  SDL_Cursor *cursor = 0;
  SDL_Event event;
  int w;
  int h;

  bool quit = false;
  babe_surface = SDL_LoadBMP("res/car.bmp");
  if (!babe_surface) return die("surfaces are missing");
  w = babe_surface->w / 2;
  h = babe_surface->h / 2;
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) return die("can't init SDL");
  if (TTF_Init() < 0) return die("can't init fonts");
  if (!(font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 72))) return die("can't load the font");
  SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_RESIZABLE, &window, &renderer);

  babe = SDL_CreateTextureFromSurface(renderer, babe_surface);
  if (!babe) return die("functionality is limited");
  SDL_FreeSurface(babe_surface);

  cursor_surface = SDL_LoadBMP("res/cursor.bmp");
  if ((cursor = SDL_CreateColorCursor(cursor_surface, 5, 7)) == 0) return die("cursor nope'd");
  SDL_FreeSurface(cursor_surface);
  SDL_SetCursor(cursor);
  SDL_Cursor *cursors[SDL_NUM_SYSTEM_CURSORS];
  for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i) {
    cursors[i] = SDL_CreateSystemCursor(i);
  }
  SDL_SetCursor(cursor);

  SDL_Rect mouse_follow_rect = {10, 10};
  mouse_follow_word = make_text(renderer, font, "*", (SDL_Color) {255, 255, 255, 255}, &mouse_follow_rect);

  u32 frame_delay = 16;
  int mouse_x, mouse_y;

  read_slideshow_file(SIGCONT);
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
        frame_delay = 0;
        break;
      }
      switch ((SDL_EventType) event.type) {
        case SDL_WINDOWEVENT: {
          do_window(&event, &frame_delay, &w, &h);
          break;
        }
        case SDL_KEYDOWN: {
          quit = do_keydown(&event, show, &frame_delay);
          break;
        }
        case SDL_MOUSEWHEEL: {
          show->index += -event.wheel.y;
          break;
        }
        default:
          break;
      }
      show->index = show->index < 0 ? 0 : min(show->index, count(show->slides) - 1);
    }
    
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, babe, 0, 0);

    slide_show *show_baby = show;

    draw_slide(renderer, w, h, show_baby);

    SDL_GetMouseState(&mouse_x, &mouse_y);
    mouse_follow_rect.x = mouse_x + 20;
    mouse_follow_rect.y = mouse_y + 20;

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderCopy(renderer, mouse_follow_word, 0, &mouse_follow_rect);
    SDL_RenderPresent(renderer);
    SDL_Delay(frame_delay);
  }

  TTF_CloseFont(font);
  SDL_FreeCursor(cursor);
  for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i) {
    SDL_FreeCursor(cursors[i]);
  }
  SDL_DestroyTexture(mouse_follow_word);
  SDL_DestroyTexture(babe);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  return EXIT_SUCCESS;
}

void quit(void) {
  waitpid(-1, 0, WNOHANG);
  SDL_Quit();
  TTF_Quit();
}

void read_slideshow_file(int signal) {
  info("received %s. reading %s", strsignal(signal), slide_show_file);
  if (slide_show_file) {
    FILE *f = fopen(slide_show_file, "r");
    char *content = 0;
    size_t total = 0;
    char *next;
    bool done = false;
    while(!done) {
      next = grow_by(content, read_len);
      size_t rc = fread(next, sizeof(char), read_len, f);
      total += rc;
      if (rc == 0) done = true;
    }
    content[total] = 0;
    fclose(f);
    if ((show = make_slides(content)) == 0) {
      die("this file sucks");
      exit(EXIT_FAILURE);
    }
  }
}

#pragma clang diagnostic pop

