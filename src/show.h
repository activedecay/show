//
// Created by justin on 1/12/19.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored \
        "-Wunknown-pragmas"
#pragma ide diagnostic ignored  \
        "OCUnusedMacroInspection"
#pragma ide diagnostic ignored \
        "OCUnusedGlobalDeclarationInspection"

#ifndef slideshow_h
#define slideshow_h

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "lib/uthash.h"
#include "lib/stretchy.h"

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t
#define s64 int64_t
#define s32 int32_t
#define s16 int16_t
#define s8 int8_t

#define internal static
#define local_storage static

#define LEN(a) sizeof(a) / sizeof(a[0])

#define err(format, ...) ({ \
    fprintf (stderr, format, ## __VA_ARGS__); })
#define error(format, ...) ({ \
    fprintf (stderr, format, ## __VA_ARGS__); \
    fprintf (stderr, "\n"); })
#define id(format, ...) ({ \
    fprintf (stdout, format, ## __VA_ARGS__); })
#define info(format, ...) ({ \
    fprintf (stdout, format, ## __VA_ARGS__); \
    fprintf (stdout, "\n"); })
//#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
#define dbg(format, ...) ({ \
    fprintf (stdout, format, ## __VA_ARGS__); })
#define debug(format, ...) ({ \
    fprintf (stdout, format, ## __VA_ARGS__); \
    fprintf (stdout, "\n"); })
#else
#define dbg(_, ...) {};
#define debug(_, ...) {};
#endif //DEBUG_ENABLED

#define max(a, b) \
  ({ __auto_type _a = (a); \
      __auto_type _b = (b); \
    _a > _b ? _a : _b; })
#define min(a, b) \
  ({ __auto_type _a = (a); \
      __auto_type _b = (b); \
    _a < _b ? _a : _b; })

#define BLACK   "\e[30m"
#define RED     "\e[31m"
#define GREEN   "\e[32m"
#define YELLOW  "\e[33m"
#define BLUE    "\e[34m"
#define MAGENTA "\e[35m"
#define CYAN    "\e[36m"
#define GRAY    "\e[37m"
#define RESET   "\e[39m"

typedef struct {
    struct linkedlist *next;
    void *val;
} linkedlist;

/* slide show */

typedef enum {
    text_slide
} item_type;

typedef struct {
    float y;
    item_type type;
    char *text;
} text_item;

typedef enum {
    normal,
    italic,
    bold,
    num_styles
} font_style;

static struct {
    font_style s;
    char *name;
} styles[] = {
    {normal, "normal"},
    {italic, "italic"},
    {bold,   "bold"},
};

typedef enum {
    sans,
    serif,
    mono,
    script,
    num_families
} font_family;

static struct {
    font_family f;
    char *name;
} family[] = {
    {sans,   "sans"},
    {serif,  "serif"},
    {mono,   "mono"},
    {script, "script"},
};

typedef enum {
    left,
    center,
    right,
    num_alignments
} align_text;

static struct {
    align_text a;
    char *name;
} alignments[] = {
    {left,   "left"},
    {center, "center"},
    {right,  "right"},
};

typedef struct {
    float size;
    font_style style;
    font_family family;
    align_text align;
    float line_height;
    float margins_x;
    SDL_Color fg_color;
    char *name;
} style_item;

typedef struct {
    char *name;
    style_item *style;
    UT_hash_handle hh; /* makes this structure hashable */
} style_hash;

typedef struct {
    SDL_Color bg_color;
    char *title;
    text_item **items;
    style_item **styles;
} slide_item;

typedef struct {
    int index;
    slide_item **slides;
} slide_show;

static SDL_Color cf4(float fr, float fg, float fb, float fa) {
  SDL_Color c = {
      (u8) (255.0f * fr),
      (u8) (255.0f * fg),
      (u8) (255.0f * fb),
      (u8) (255.0f * fa),
  };
  return c;
}

typedef struct {
    char *id;
    char *filename;
    UT_hash_handle hh; /* makes this structure hashable */
} font;

font *find_font(font *fonts, char *name);

slide_show *init_slides(slide_show *previous_show, char *content);

void render_slide(SDL_Renderer *renderer, int w, int h,
                  slide_show *show, font *fonts);

SDL_Texture *
texturize_text(SDL_Renderer *renderer, TTF_Font *font, char *string,
               SDL_Color fg, SDL_Rect *r, SDL_BlendMode mode);

typedef void (*render_slide_ptr)(SDL_Renderer *renderer, int w, int h,
                                 slide_show *show, font *fonts);

typedef SDL_Texture *(*texturize_text_ptr)(SDL_Renderer *renderer,
                                           TTF_Font *font, char *string,
                                           SDL_Color fg, SDL_Rect *r, SDL_BlendMode mode);

typedef slide_show *(*init_slides_ptr)(slide_show *previous_show, char *content);

#endif //slideshow_h

#pragma clang diagnostic pop