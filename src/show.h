//
// Created by justin on 1/12/19.
//

#ifndef slideshow_h
#define slideshow_h

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <values.h>
#include <assert.h>
#include <dirent.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "lib/uthash.h"
#include "lib/stretchy.h"
#include "lib/csapp.h"

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
#define DEBUG_ENABLED
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

typedef struct linkedlist linkedlist;
struct linkedlist {
    struct linkedlist *next;
    void *data;
};

typedef struct image_item image_item;
struct image_item {
  char *text;
  SDL_Texture *image;
  int width, height;
};

/* slide show */

#define SHADOW_DISTANCE 2

typedef enum {
    normal,
    italic,
    bold,
    num_styles
} font_style;

typedef enum {
    sans,
    serif,
    mono,
    script,
    num_families
} font_family;

typedef enum {
    left,
    center,
    right,
    num_alignments
} align_text;

static struct {
    font_style s;
    char *name;
} styles[] = {
    {normal, "normal"},
    {italic, "italic"},
    {bold,   "bold"},
};

static struct {
    font_family f;
    char *name;
} family[] = {
    {sans,   "sans"},
    {serif,  "serif"},
    {mono,   "mono"},
    {script, "script"},
};

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
    SDL_Color shadow_color;
    char *name;
    UT_hash_handle hh;
} style_item;

typedef struct {
    float x, y;
} point;

typedef struct {
  float x, y;
  float w, h;
} rect;


typedef enum grocer_types {
  text_t_item,
  image_t_item,
} grocer_type;

typedef struct {
    grocer_type type;
    style_item *style;
    point *pos;
    bool image_texture;
    int w, h;
    rect src_rect; // integer based (for cropping texture space)
    rect dst_rect; // integer based (for cropping screen space)
    union {
        char *text;
        SDL_Texture *image;
    } item;
} item_grocer;

typedef struct slide_item slide_item;
struct slide_item {
    char *title;
    SDL_Color bg_color;
    item_grocer **grocery_items;
    slide_item **using;
};

typedef struct {
    char *id;
    slide_item *slide;
    UT_hash_handle hh;
} template_slide;

typedef struct {
    char *id; /* image_alias - the image's alias in `. define-image [alias] [res_name]` */
    char *image_res_name; /* res_name - the image's /res/ filename in `. define-image [alias] [res_name]` */
    UT_hash_handle hh;
} image_hash;

typedef struct {
    int index;
    template_slide *template_slides;
    slide_item **slides;
    point **positions;
    image_hash *images;
    style_item **styles;
} slide_show;

/** our colors are 0f-1.0f, SDL's are 0-255. so we convert by multiplying */
static SDL_Color cf4(float r, float g, float b, float a) {
  SDL_Color c = {
      (u8) (255.0f * r),
      (u8) (255.0f * g),
      (u8) (255.0f * b),
      (u8) (255.0f * a),
  };
  return c;
}

typedef struct {
    char *id;
    char *filename;
    UT_hash_handle hh; /* makes this structure hashable */
} font;

slide_show *default_show();

void draw_slide_items(SDL_Renderer *, int, int,
                      const font *, const slide_item *, linkedlist *, grocer_type);

SDL_Texture *get_texture_from_image(SDL_Renderer *, char *, image_item *);

/* fancy defines for robustly making a fuction pointer
 * whose parameter list can change at will */

/* find font */
#define FUNCTION_FF(fun) font *fun(const font *, char *)
typedef FUNCTION_FF((*find_font_ptr));
FUNCTION_FF(find_font);

#define FUNCTION_IS(fun) slide_show *fun(int, style_item **, char *)
typedef FUNCTION_IS((*init_slides_ptr));
FUNCTION_IS(init_slides);

#define FUNCTION_FS(fun) int fun(slide_show *, style_item **)
typedef FUNCTION_FS((*free_show_ptr));
FUNCTION_FS(free_show);

#define FUNCTION_RS(fun) void fun(SDL_Renderer *, int, int,        \
        slide_show *, font *, linkedlist *, bool)
typedef FUNCTION_RS((*render_slide_ptr));
FUNCTION_RS(render_slide);

#define FUNCTION_TT(fun) SDL_Texture * fun(SDL_Renderer *renderer, \
        TTF_Font *font, char *string, SDL_Color fg,                \
        SDL_Rect *rect, SDL_BlendMode mode)
typedef FUNCTION_TT((*texturize_text_ptr));
FUNCTION_TT(texturize_text);

typedef struct {
    init_slides_ptr init_slides_func;
    free_show_ptr free_show_func;
    render_slide_ptr render_slide_func;
    texturize_text_ptr texturize_text_func;
    find_font_ptr find_font_func;
    void *it;
} game_lib;

#endif //slideshow_h
