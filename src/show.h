//
// Created by justin on 1/12/19.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#ifndef slideshow_h
#define slideshow_h

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

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

typedef enum {text_slide} item_type;

typedef struct {
    item_type type;
    int y;
    int line_cnt;
    char *text_lines[];
} text_item;

typedef struct {
    int id;
    u32 bg_color;
    void **items;
} slide;

typedef struct {
    int count;
    slide *slides[];
} slide_show;

static char *const res_dir = "./res/";

#endif //slideshow_h

#pragma clang diagnostic pop