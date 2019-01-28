//
// Created by justin on 1/12/19.
//

#include <values.h>
#include "show.h"
#include "lib/stretchy.h"

void set_fam(style_item *style, const char *token) {
  for (int i = 0; i < num_families; ++i)
    if (strcmp(family[i].name, token) == 0) {
      style->family = family[i].f;
      break;
    }
}
char *get_fam(style_item *style) {
  for (int i = 0; i < num_families; ++i)
    if (family[i].f == style->family) return family[i].name;
  return 0;
}

void set_style(style_item *style, const char *token) {
  for (int i = 0; i < num_styles; ++i)
    if (strcmp(styles[i].name, token) == 0) {
      style->style = styles[i].s;
      break;
    }
}
char *get_style(style_item *style) {
  for (int i = 0; i < num_styles; ++i)
    if (styles[i].s == style->style) return styles[i].name;
}

void set_align(style_item *style, const char *token) {
  for (int i = 0; i < num_alignments; ++i)
    if (strcmp(alignments[i].name, token) == 0) {
      style->align = alignments[i].a;
      break;
    }
}

slide_show *init_slides(char *content) {
  slide_show *the_show = 0;
  int command_starter = '.';
  int slide_starter = '#';

  if (content) {
    the_show = calloc(1, sizeof(slide_show));
    slide_item *slide = 0;
    style_item *style = 0;
    text_item *item = 0;
    float yyy = MINFLOAT;

    style_item *initial;
    initial = calloc(1, sizeof(style_item));
    initial->style = normal;
    initial->family = sans;
    initial->align = center;
    initial->size = .1f;
    float initialY = .5f;

    char *line_tokenizer, *space_tokenizer;
    char *line = strtok_r(content, "\n", &line_tokenizer);
    while (line) {
      info("(line ): %s", line);
      if (line[0] == command_starter) {
        char *token = strtok_r(line, " ", &space_tokenizer);
        while (token) {
          if (strcmp("font", token) == 0) {
            style = calloc(1, sizeof(style_item));
            token = strtok_r(0, " ", &space_tokenizer);
            style->size = strtof(token, 0); // require `. font [size]`
            while (token) {
              set_fam(style, token);
              set_style(style, token);
              set_align(style, token);
              token = strtok_r(0, " ", &space_tokenizer);
            }
            continue; // we ate all tokens on the font line
          } else if (strcmp("y", token) == 0) {
            token = strtok_r(0, " ", &space_tokenizer);
            yyy = strtof(token, 0); // require `. y [size]`
          }
          token = strtok_r(0, " ", &space_tokenizer);
        }

      } else if (line[0] == slide_starter) { /* start of a slide */

        style = 0;
        yyy = 0;
        strtok_r(line, " ", &space_tokenizer); // eat space tokens
        char *title = strtok_r(0, "\n", &space_tokenizer);
        slide = calloc(1, sizeof(slide_item));
        slide->title = title;
        slide->bg_color = cf4(0, 0, 0, .8);
        push(the_show->slides, slide);

      } else { /* we have text*/

        if (!slide) {
          error("no slide before -- %s", line);
          continue;
        }

        item = calloc(1, sizeof(text_item));
        push(slide->items, item);
        item->fg_color = cf4(1, 1, 1, 1);
        item->type = text_slide;
        item->y = yyy == MINFLOAT ? initialY : yyy;
        size_t len = strlen(line);
        item->text = malloc(len * sizeof(char) + 1);
        memcpy(item->text, line, len + 1);
        push(slide->styles, style ? : initial);
      }
      line = strtok_r(0, "\n", &line_tokenizer);
    }
  } else {
    the_show = calloc(1, sizeof(slide_show));

    char ***slides = 0;

    int slide_cnt = 3;
    int line_cnt = 2;

    for (int i = 0; i < slide_cnt; ++i) {
      char **arr = 0;
      for (int j = 0; j < line_cnt; ++j) {
        char *line;
        char *str;
        size_t len;
        line = "Oh, baby! Lookit dem!";
        len = strlen(line);
        str = malloc(len * sizeof(char) + 1);
        memcpy(str, line, len + 1);
        push(arr, str);
      }
      push(slides, arr);
    }
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

        style->style = (i + j) % num_styles;
        style->family = (i + j) % num_families;
        style->align = (i + j) % num_alignments;
        style->size = .1f;

        text_item *item = calloc(1, sizeof(text_item));
        push(slide->items, item);

        item->fg_color = cf4(1, 1, 1, 1);
        item->type = text_slide;
        item->text = str;
        item->y = .5;
      }
    }
  }

  return the_show;
}

void render_slide(SDL_Renderer *renderer, int w, int h, slide_show *show_baby) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  slide_item *current_slide = show_baby->slides[show_baby->index];
  SDL_Color slide_bg = current_slide->bg_color;
  SDL_SetRenderDrawColor(renderer, slide_bg.r, slide_bg.g, slide_bg.b, slide_bg.a);
  SDL_RenderFillRect(renderer, 0);

  for (int slide_i = 0; slide_i < count(current_slide->items); ++slide_i) {
    text_item *item = current_slide->items[slide_i];
    style_item *style = current_slide->styles[slide_i];
    SDL_Rect box = {0, (int) (h * item->y)};
    char *fam = get_fam(style);
    char *sty = get_style(style);
    char font_idx[strlen(fam) + strlen(sty) + 1];
    strcpy(font_idx, fam);
    strcat(font_idx, sty);
    TTF_Font *f = TTF_OpenFont(find_font(font_idx)->filename, (int) (style->size * h));
    SDL_Texture *slide_text =
        texturize_text(renderer, f,
                       item->text,
                       item->fg_color,
                       &box);
    box.y -= -(box.h * 2 / 3 + slide_i * box.h);
    int margins_x = (int) (w * .05f);
    switch (style->align) {
      default:
      case left:
        box.x += margins_x;
        break;
      case center:
        box.x += w / 2 - box.w / 2;
        break;
      case right:
        box.x = w - box.w - margins_x;
        break;
    }
    TTF_CloseFont(f);
    SDL_RenderCopy(renderer, slide_text, 0, &box);
    SDL_DestroyTexture(slide_text);
  }
}

SDL_Texture *texturize_text(SDL_Renderer *renderer, TTF_Font *font,
                            char *string, SDL_Color fg, SDL_Rect *r) {
  SDL_Surface *t;
  if (!(t = TTF_RenderText_Blended(font, string, fg))) return 0;
  SDL_SetSurfaceBlendMode(t, SDL_BLENDMODE_BLEND);
  SDL_Texture *words = SDL_CreateTextureFromSurface(renderer, t);
  SDL_FreeSurface(t);
  SDL_QueryTexture(words, 0, 0, &r->w, &r->h);
  return words;
}

font *add_font(char *name, char *filepath) {
  font *found = find_font(name);
  if (found) return found;
  found = malloc(sizeof(font));
  found->id = name;
  found->filename = filepath;
  HASH_ADD_STR(fonts, id, found);
}

font *find_font(char *name) {
  font *s;
  HASH_FIND_STR(fonts, name, s);
  return s;
}
