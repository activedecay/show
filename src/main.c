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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

void lol(void) {
  error("lol you ran out of memory!");
}

#pragma clang diagnostic pop

#define STRETCHY_BUFFER_OUT_OF_MEMORY lol;

#include "lib/stretchy.h"
#include "lib/ino.h"
#include "lib/csapp.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF

#include "../src/lib/stb_image.h"

/* shared object functions */
init_slides_ptr init_slides_func = 0;
render_slide_ptr render_slide_func = 0;
texturize_text_ptr texturize_text_func = 0;
find_font_ptr find_font_func = 0;

/* globals */
font *fonts = 0;
sem_t show_sem;
char *slide_show_file = 0;
slide_show *show;
size_t read_len = 1024 * 4;
sem_t game_sem;
void *game_lib;


bool do_keydown(SDL_Event *event, slide_show *show,
                uint32_t *frame_delay);

font *add_font(char *, char *);

int die(char *s);

SDL_Cursor *get_cursor();

SDL_Texture *get_texture_from_image(SDL_Renderer *, char *);

void do_window(SDL_Event *event, uint32_t *frame_delay,
               int *w, int *h, bool *in_frame);

void load_game_library(void);

void quit(void);

void read_slideshow_file(void);

void *watch_library(void *library);

void *watch_slideshow_file(void *slide_show_file);

int
main(int argc, char *argv[]) {
  if (argc < 2) return die("usage: path/to/show.md");
  atexit(quit);
  // mutual exclusion specifically for game lib thread and main
  Sem_init(&game_sem, 0, 1);
  // mutual exclusion specifically for slide file thread and main
  Sem_init(&show_sem, 0, 1);

  // make sure we have a game library before forking a thread
  load_game_library();

  pthread_t tidp;
  slide_show_file = argc < 2 ? 0 : argv[1];
  Pthread_create(&tidp, 0, watch_slideshow_file, slide_show_file);
  Pthread_create(&tidp, 0, watch_library, "lib/libslider.so");

  SDL_Window *window = 0;
  SDL_Renderer *renderer = 0;
  TTF_Font *font = 0;
  SDL_Texture *mouse_follow_word = 0;
  SDL_Event event;
  int w = 960;
  int h = 540;

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    return die("can't init SDL");
  if (TTF_Init() < 0) return die("can't init fonts");
  if (!(font = TTF_OpenFont("./res/FreeSans.ttf", 72)))
    return die("can't load the font");
  SDL_CreateWindowAndRenderer(
      w, h, SDL_WINDOW_RESIZABLE, &window, &renderer);

  SDL_Texture *babe = get_texture_from_image(
      renderer, "./res/blue-lambo.jpg");

  SDL_Cursor *cursor = get_cursor();

  SDL_Rect mouse_follow_rect = {10, 10};
  mouse_follow_word = texturize_text_func(
      renderer, font, "*", (SDL_Color) {255, 255, 255, 255},
      &mouse_follow_rect, SDL_BLENDMODE_BLEND);

  /* note @Evict from main */
  add_font("sansnormal", "./res/FreeSans.ttf");
  add_font("sansbold", "./res/FreeSansBold.ttf");
  add_font("sansitalic", "./res/FreeSansOblique.ttf");
  add_font("serifnormal", "./res/FreeSerif.ttf");
  add_font("serifbold", "./res/FreeSerifBold.ttf");
  add_font("serifitalic", "./res/FreeSerifItalic.ttf");
  add_font("mononormal", "./res/FreeMono.ttf");
  add_font("monobold", "./res/FreeMonoBold.ttf");
  add_font("monoitalic", "./res/FreeMonoOblique.ttf");
  add_font("scriptnormal", "./res/AlexBrush-Regular.ttf");
  add_font("scriptitalic", "./res/AlexBrush-Regular.ttf");
  add_font("scriptbold", "./res/AlexBrush-Regular.ttf");

  read_slideshow_file();

  if (!show->slides)
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
      switch ((SDL_EventType) event.type) {
        case SDL_WINDOWEVENT: {
          do_window(&event, &frame_delay, &w, &h, &in_frame);
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
      show->index = show->index < 0 ? 0
          : min(show->index, count(show->slides) - 1);
    }

//    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
//    SDL_RenderClear(renderer);
//    SDL_RenderCopy(renderer, babe, 0, 0);

    if (show->slides) render_slide_func(renderer, w, h, show, fonts);

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

  TTF_CloseFont(font);
  SDL_FreeCursor(cursor);
  SDL_DestroyTexture(mouse_follow_word);
  SDL_DestroyTexture(babe);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  return EXIT_SUCCESS;
}


SDL_Texture *get_texture_from_image(SDL_Renderer *renderer, char *filename) {
  SDL_Surface *image_surface = 0;
  int image_width;
  int image_height;
  int n_chans;
  int desire_rgba = 4;
  stbi_uc *image =
      stbi_load(filename, &image_width,
                &image_height, &n_chans, desire_rgba);
  if (!image) {
    error("image load failure: %s", stbi_failure_reason());
    return 0;
  }
  int bits_ppix = n_chans * 8;
  image_surface = SDL_CreateRGBSurfaceWithFormatFrom(
      image, image_width, image_height, bits_ppix,
      image_width * 4, SDL_PIXELFORMAT_ABGR8888);
  if (!image_surface) {
    error("could not create surface from image file");
    return 0;
  }

  SDL_Texture *image_texture =
      SDL_CreateTextureFromSurface(renderer, image_surface);
//  SDL_UpdateTexture(image_texture, 0, image, image_width * 4);
  if (!image_texture) {
    error("could not create texture from image surface");
    return 0;
  }
  SDL_FreeSurface(image_surface);
  stbi_image_free(image);
  return image_texture;
}

void quit(void) {
  SDL_Quit();
  TTF_Quit();
}

void read_slideshow_file() {
  if (slide_show_file) {
    FILE *f = fopen(slide_show_file, "r");
    char *content = 0;
    size_t total = 0;
    char *next;
    bool done = false;
    while (!done) {
      next = grow_by(content, read_len);
      size_t rc = fread(next, sizeof(char), read_len, f);
      total += rc;
      if (rc == 0) done = true;
    }
    content[total] = 0;
    fclose(f);

    P(&show_sem);
    if ((show = init_slides_func(show, content)) == 0) {
      // @Leak! we use these strings everywhere, though.
      // so we can't really free(content) yet.
      die("this show file sucks!");
      exit(EXIT_FAILURE);
    }
    V(&show_sem);
  }
}

font *add_font(char *name, char *filepath) {
  font *found;
  HASH_FIND_STR(fonts, name, found);
  if (found) return found;
  found = Malloc(sizeof(font));
  found->id = name;
  found->filename = filepath;
  HASH_ADD_STR(fonts, id, found);
}

SDL_Cursor *get_cursor() {
  SDL_Cursor *cursor = 0;
  SDL_Surface *cursor_surface = 0;
  int image_width;
  int image_height;
  int n_chans;
  int desire_rgba = 4;
  stbi_uc *cursor_image =
      stbi_load("./res/cursor.png", &image_width,
                &image_height, &n_chans, desire_rgba);
  int bits_ppix = n_chans * 8;
  cursor_surface = SDL_CreateRGBSurfaceWithFormatFrom(
      cursor_image, image_width, image_height, bits_ppix,
      image_width * 4, SDL_PIXELFORMAT_ABGR8888);
  if ((cursor = SDL_CreateColorCursor(cursor_surface, 5, 7)) == 0) {
    error("can't create a cursor");
  }
  SDL_FreeSurface(cursor_surface);
  SDL_SetCursor(cursor);
  stbi_image_free(cursor_image);
  return cursor;
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

void load_game_library(void) {
  P(&game_sem);
  if (game_lib) dlclose(game_lib);

  if (!(game_lib = dlopen("libslider.so", RTLD_LAZY))) {
    fprintf(stderr, "bitch %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  dlerror(); // clear existing errors

  init_slides_func = dlsym(game_lib, "init_slides");
  render_slide_func = dlsym(game_lib, "render_slide");
  texturize_text_func = dlsym(game_lib, "texturize_text");
  info("shh s'alibrary %p", game_lib);
  V(&game_sem);

  char *error;
  if ((error = dlerror()) != 0) {
    fprintf(stderr, "todo lasagna %s\n", error);
    exit(EXIT_FAILURE);
  }

  read_slideshow_file();
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
      (*frame_delay) = 200;
      break;
    }
  }
}

int die(char *s) {
  error("sorry, charlie: %s", s);
  return EXIT_FAILURE;
}

void *watch_slideshow_file(void *slide_show_file) {
  inotify(slide_show_file, read_slideshow_file); // child
}

void *watch_library(void *library) {
  inotify(library, load_game_library); // child
}

#pragma clang diagnostic pop
