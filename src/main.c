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

void lol(void) {
  error("lol you ran out of memory");
}

#define STRETCHY_BUFFER_OUT_OF_MEMORY lol;

#include "lib/stretchy.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wimplicit-int"
#pragma ide diagnostic ignored "OCDFAInspection"

char *Sans = "/usr/share/fonts/truetype/freefont/FreeSans.ttf";
char *SansBold = "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf";
char *SansOblique = "/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf";
char *Serif = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf";
char *SerifBold = "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf";
char *SerifItalic = "/usr/share/fonts/truetype/freefont/FreeSerifItalic.ttf";
char *Mono = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";
char *MonoBold = "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf";
char *MonoOblique = "/usr/share/fonts/truetype/freefont/FreeMonoOblique.ttf";


int
die(char *s) {
  error("sorry, charlie: %s", s);
  return EXIT_FAILURE;
}

int
load_game_library() {
  void *game_lib;
  if (!(game_lib = dlopen("libslider.so", RTLD_LAZY))) {
    fprintf(stderr, "bitch %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  dlerror(); // clear existing errors

//  advance_time_ptr =     dlsym(game_lib, "advance_time");

  char *error;
  if ((error = dlerror()) != 0) {
    fprintf(stderr, "lasagna %s\n", error);
    exit(EXIT_FAILURE);
  }
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

SDL_Texture *texturize_text(SDL_Renderer *renderer, TTF_Font *font, char *string, SDL_Color fg, SDL_Rect *r) {
  SDL_Surface *t;
  if (!(t = TTF_RenderText_Blended(font, string, fg))) return 0;
  SDL_SetSurfaceBlendMode(t, SDL_BLENDMODE_BLEND);
  SDL_Texture *words = SDL_CreateTextureFromSurface(renderer, t);
  SDL_FreeSurface(t);
  SDL_QueryTexture(words, 0, 0, &r->w, &r->h);
  return words;
}


slide_show *init_slides() {
  char ***slides = 0;

  int slide_cnt = 3;
  int line_cnt = 2;

  for (int i = 0; i < slide_cnt; ++i) {
    char **arr = 0;
    for (int j = 0; j < line_cnt; ++j) {
      char *line;
      char *str;
      size_t len;
      line = "oh";
      len = strlen(line);
      str = malloc(len * sizeof(char) + 1);
      memcpy(str, line, len + 1);
      char x[11];
      char *y = SDL_itoa(i + j, x, 10);
      push(arr, str);
    }
    push(slides, arr);
  }

  slide_show *the_show = calloc(1, sizeof(slide_show));
  slide_item *slide;
  style_item *style;

  for (int i = 0; i < count(slides); ++i) {

    slide = calloc(1, sizeof(slide_item));
    push(the_show->slides, slide);

    slide->bg_color = cf4(0, 0, 0, .8);

    for (int j = 0; j < count(slides[i]); ++j) {
      char *str = slides[i][j];

      style = calloc(1, sizeof(style_item));
      push(slide->styles, style);

      style->style =  (i + j) % num_styles;
      style->family = (i + j) % num_families;
      style->align = (i + j) % num_alignments;
      style->size =   .1f;

      text_item *item = calloc(1, sizeof(text_item));
      push(slide->items, item);

      item->fg_color =  cf4(1, 1, 1, 1);
      item->type =      text_slide;
      item->text =      str;
      item->y =         .5;
    }
  }

  return the_show;
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
  if (!(font = TTF_OpenFont(SerifItalic, 72))) return die("can't load the font");
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
  mouse_follow_word = texturize_text(renderer, font, "*", (SDL_Color) {0, 0, 0}, &mouse_follow_rect);

  u32 frame_delay = 16;
  int mouse_x, mouse_y;

  slide_show *show;
  if ((show = init_slides()) == 0) return die("what? no show");

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
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    slide_item *current_slide = show->slides[show->index];
    SDL_Color slide_bg = current_slide->bg_color;
    SDL_SetRenderDrawColor(renderer, slide_bg.r, slide_bg.g, slide_bg.b, slide_bg.a);
    SDL_RenderFillRect(renderer, 0);

    for (int i = 0; i < count(current_slide->items); ++i) {
      text_item *item = current_slide->items[i];
      style_item *style = current_slide->styles[i];
      SDL_Rect slide_text_rect = {0, (int) (h * item->y)};
      TTF_Font *f = TTF_OpenFont(Serif, (int) (style->size * h));
      SDL_Texture *slide_text =
          texturize_text(renderer, f,
                         item->text,
                         item->fg_color,
                         &slide_text_rect);
      slide_text_rect.y -= slide_text_rect.h * 2/3 - i*slide_text_rect.h;
      switch(style->align) {
        default:
        case left:
          slide_text_rect.x += (int)(w * .10f);
          break;
        case center:
          slide_text_rect.x += w/2 - slide_text_rect.w/2;
          break;
        case right:
          slide_text_rect.x = w - slide_text_rect.w - (int)(w * .10f);
          break;
      }
      TTF_CloseFont(f);
      SDL_RenderCopy(renderer, slide_text, 0, &slide_text_rect);
      SDL_DestroyTexture(slide_text);
    }

    SDL_GetMouseState(&mouse_x, &mouse_y);
    mouse_follow_rect.x = mouse_x;
    mouse_follow_rect.y = mouse_y;
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

#pragma clang diagnostic pop
#pragma clang diagnostic pop