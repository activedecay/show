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

#include "show.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wimplicit-int"
#pragma ide diagnostic ignored "OCDFAInspection"

char *font_file = "/usr/share/fonts/truetype/freefont/FreeSans.ttf";

int
die(char *s) {
  error("sorry, charlie: %s", s);
  return EXIT_FAILURE;
}

int
load_game_library() {
  void *game_lib;
  if (!(game_lib = dlopen("libgame.so", RTLD_LAZY))) {
    fprintf(stderr, "bitch %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  dlerror(); // clear existing errors

//  advance_time_ptr =     dlsym(game_lib, "advance_time");
//  make_controllers_ptr = dlsym(game_lib, "make_controllers");
//  make_game_ptr =        dlsym(game_lib, "make_game");
//  map_text_to_controller_ptr = dlsym(game_lib, "map_text_to_controller");
//  print_game_ptr =       dlsym(game_lib, "print_game");
//  process_inputs_ptr =   dlsym(game_lib, "process_inputs");
//  randomize_inputs_ptr = dlsym(game_lib, "randomize_inputs");
//  sort_ptr =             dlsym(game_lib, "sort");

  char *error;
  if ((error = dlerror()) != 0) {
    fprintf(stderr, "lasagna %s\n", error);
    exit(EXIT_FAILURE);
  }
}

int
select_bmp_files(const struct dirent *d) {
  return strstr(d->d_name, ".bmp") != 0;
}

void quit(void) {
  SDL_Quit();
  TTF_Quit();
}

int
main(int argc, char *argv[]) {
  load_game_library();
  atexit(quit);
  SDL_Window *window = 0;
  SDL_Renderer *renderer = 0;
  SDL_Surface *babe_surface = 0;
  SDL_Texture *babe = 0;
  TTF_Font *font = 0;
  SDL_Texture *words = 0;
  SDL_Surface *cursor_surface = 0;
  SDL_Cursor *cursor = 0;
  SDL_Event event;

  bool quit = false;
  bool reinitializeTexture = true;
  babe_surface = SDL_LoadBMP("res/car.bmp");
  if (!babe_surface) return die("surfaces are missing");
  int w = babe_surface->w;
  int h = babe_surface->h;
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) return die("can't init SDL");
  if (TTF_Init() < 0) return die("can't init fonts");
  if (!(font = TTF_OpenFont(font_file, 72))) die("can't load the font");
  window = SDL_CreateWindow("cute!", 0, 0, w / 2, h / 2, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  SDL_SetWindowPosition(window,
                        SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);


  babe = SDL_CreateTextureFromSurface(renderer, babe_surface);
  if (!babe) return die("functionality is limited");
  SDL_FreeSurface(babe_surface);
  SDL_QueryTexture(babe, 0, 0, &w, &h);
  SDL_Texture *stream = SDL_CreateTexture(renderer,
                                          SDL_PIXELFORMAT_RGBA8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          w,
                                          h);
  SDL_SetTextureBlendMode(stream, SDL_BLENDMODE_BLEND);
  const SDL_Rect *rect = 0;
  int pitch = w * 4;
  size_t size = (size_t) w * h;
  size_t len = size * 4;
  void *pixels = mmap(0, len, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);
  SDL_UpdateTexture(stream, rect, pixels, pitch);

  cursor_surface = SDL_LoadBMP("res/cursor.bmp");
  if ((cursor = SDL_CreateColorCursor(cursor_surface, 5, 7)) == 0) die("cursor nope'd");
  SDL_FreeSurface(cursor_surface);
  SDL_SetCursor(cursor);
  SDL_Cursor *cursors[SDL_NUM_SYSTEM_CURSORS];
  for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i) {
    cursors[i] = SDL_CreateSystemCursor(i);
  }
  SDL_SetCursor(cursor);

  SDL_Color cc = {0, 0, 0};
  SDL_Surface *text_surface;
  if (!(text_surface = TTF_RenderText_Blended(font, "hey baby!", cc)))
    die("use your words!");
  words = SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_FreeSurface(text_surface);
  SDL_Rect word_dest;
  word_dest.x = 10;
  word_dest.y = 10;
  SDL_QueryTexture(words, 0, 0, &word_dest.w, &word_dest.h);
  u32 colors[] = {
      0xffff0033,
      0x00ffff33,
      0xff00ff33,
      0xff000033,
      0x00ff0033,
      0x0000ff33,
  };
  int color = 0;
  u32 frame_delay = 16;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch((SDL_EventType) event.type) {
        default:break;
        case SDL_QUIT: {
          frame_delay = 0;
          quit = true;
          break;
        }
        case SDL_WINDOWEVENT: {
          switch((SDL_WindowEventID) event.window.event) {
            default:break;
            case SDL_WINDOWEVENT_SIZE_CHANGED: {
              w = event.window.data1;
              h = event.window.data2;
              pitch = w * 4;
              size = (size_t) w * h;
              len = size * 4;
              munmap(pixels, len);
              SDL_DestroyTexture(stream);
              stream = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         w,
                                         h);
              SDL_SetTextureBlendMode(stream, SDL_BLENDMODE_BLEND);
              pixels = mmap(0, len, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS,
                            -1, 0);
              break;
            }
            case SDL_WINDOWEVENT_ENTER:
            case SDL_WINDOWEVENT_FOCUS_GAINED: {
              frame_delay = 16;
              break;
            }
            case SDL_WINDOWEVENT_LEAVE:
            case SDL_WINDOWEVENT_FOCUS_LOST: {
              frame_delay = 200;
              break;
            }
          }
          reinitializeTexture = true;
          color = (color + 1) % 6;
          break;
        }
        case SDL_KEYDOWN: {
          if (event.key.keysym.sym == SDLK_q) {
            quit = true;
          } else if (event.key.keysym.sym == SDLK_SLASH) {
            SDL_CaptureMouse(true);
          } else if (event.key.keysym.sym == SDLK_1) {
            SDL_CaptureMouse(false);
          } else if (event.key.keysym.sym == SDLK_2) {
            SDL_SetCursor(cursors[SDL_SYSTEM_CURSOR_IBEAM]);
          } else if (event.key.keysym.sym == SDLK_3) {
            SDL_SetCursor(cursor);
          }
          reinitializeTexture = true;
          color = (color + 1) % 6;
          break;
        }
        case SDL_MOUSEWHEEL: {
          SDL_WarpMouseInWindow(window, 10, 10);
          reinitializeTexture = true;
          color = (color + 1) % 6;
          break;
        }
      }
    }

    if (reinitializeTexture) {
      SDL_LockTexture(stream, rect, &pixels, &pitch);
      Uint32 *pix = (Uint32 *) pixels;
      for (Uint32 i = 0; i < size; ++i) {
        pix[i] = colors[color];
      }
      SDL_UpdateTexture(stream, rect, pixels, pitch);
      SDL_UnlockTexture(stream);
      reinitializeTexture = false;
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, babe, 0, 0);
    SDL_RenderCopy(renderer, stream, 0, 0);
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    //SDL_GetGlobalMouseState(&mouse_x, &mouse_y); // desktop relative
    word_dest.x += (mouse_x - word_dest.x) / 5;
    word_dest.y += (mouse_y - word_dest.y) / 5;
    SDL_RenderCopy(renderer, words, 0, &word_dest);
    SDL_RenderPresent(renderer);
    SDL_Delay(frame_delay);
  }
  munmap(pixels, len);
  TTF_CloseFont(font);
  SDL_FreeCursor(cursor);
  for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i) {
    SDL_FreeCursor(cursors[i]);
  }
  SDL_DestroyTexture(words);
  SDL_DestroyTexture(stream);
  SDL_DestroyTexture(babe);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  return EXIT_SUCCESS;
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop