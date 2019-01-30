//
// Created by justin on 1/12/19.
//

#include <values.h>
#include <assert.h>
#include "show.h"
#include "lib/stretchy.h"
#include "lib/csapp.h"

style_hash *saved_styles;

slide_show *default_show();

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

slide_show *init_slides(slide_show *previous_show, char *content) {
  slide_show *the_show = 0;
  int command_starter = '.';
  int slide_starter = '#';
  int comment_starter = ';';

  if (!content) {
    return default_show();

  } else {
    /* let's make a new show from the content */

    the_show = Calloc(1, sizeof(slide_show));
    if (previous_show) the_show->index = previous_show->index;

    style_item *defined_style = 0;

    slide_item *slide = 0;
    style_item *style = 0;
    text_item *item = 0;
    float yyy = MINFLOAT;

    style_item *initial;
    initial = Calloc(1, sizeof(style_item));
    initial->style = normal;
    initial->family = serif;
    initial->align = center;
    initial->size = .1f;
    initial->fg_color = cf4(1, 1, 1, 1);
    float initial_line_height = 1.f;
    initial->line_height = initial_line_height;
    float initial_margin_x = .05f;
    initial->margins_x = initial_margin_x;
    SDL_Color initial_bg = cf4(0, 0, 0, .8);
    float initialY = .5f;
    SDL_Color *last_bg = 0;

    char *line_tokenizer, *space_tokenizer;
    char *line = strtok_r(content, "\n", &line_tokenizer);
    while (line) {
      info("(line ): %s", line);

      if (line[0] == comment_starter) {
        /* ; comment line */

      } else if (line[0] == slide_starter) {
        /* # start of a slide */

        yyy = 0;
        strtok_r(line, " ", &space_tokenizer); // eat space tokens
        slide = Calloc(1, sizeof(slide_item));
        slide->title = strtok_r(0, "\n", &space_tokenizer);
        slide->bg_color = last_bg ? *last_bg : initial_bg;
        push(the_show->slides, slide);

      } else if (line[0] == command_starter) {
        /* . [*]... start of command */

        char *token = strtok_r(line, " ", &space_tokenizer);
        while (token) {
          if (strcmp("font", token) == 0) {
            /* . font [float] [*]... */

            style = defined_style ? : Calloc(1, sizeof(style_item));
            style->line_height = initial_line_height;
            style->margins_x = initial_margin_x;
            token = strtok_r(0, " ", &space_tokenizer);
            style->size = strtof(token, 0);
            while (token) {
              set_fam(style, token);
              set_style(style, token);
              set_align(style, token);
              token = strtok_r(0, " ", &space_tokenizer);
            }
            continue; // we ate all tokens on the font line

          } else if (strcmp("define-style", token) == 0) {
            /* . define-style [unique-name] */

            token = strtok_r(0, " ", &space_tokenizer);
            style_hash *found = 0;
            HASH_FIND_STR(saved_styles, token, found);
            if (!found) {
              defined_style = Calloc(1, sizeof(style_item));
              memcpy(defined_style, initial, sizeof(style_item));
              defined_style->name = token;
            } else {
              defined_style = found->style;
            }

          } else if (strcmp("style", token) == 0) {
            /* . style [name] */

            token = strtok_r(0, " ", &space_tokenizer);
            style_hash *found = 0;
            HASH_FIND_STR(saved_styles, token, found);
            if (found) style = found->style;
            else
              error("can't find style named %s", token);

          } else if (strcmp("save-style", token) == 0) {
            /* . save-style */

            if (defined_style) {
              style_hash *found = 0;
              found = Malloc(sizeof(style_hash));
              found->name = defined_style->name;
              found->style = defined_style;
              HASH_ADD_STR(saved_styles, name, found);
              defined_style = 0;
            } else {
              error("tried to save a style before defining one");
            }

          } else if (strcmp("y", token) == 0) {
            /* . y [float] */

            token = strtok_r(0, " ", &space_tokenizer);
            yyy = strtof(token, 0); // fixme error require `. y [float]`

          } else if (strcmp("bg", token) == 0) {
            /* . bg [float_r] [float_g] [float_b] [? alpha] */

            token = strtok_r(0, " ", &space_tokenizer);
            float r = strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float g = strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float b = strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float a = token ? strtof(token, 0) : 1;
            slide->bg_color = cf4(r, g, b, a);
            last_bg = &slide->bg_color;

          } else if (strcmp("color", token) == 0) {
            /* text color [float_r] [float_g] [float_b] */

            style = defined_style ? : memcpy(Calloc(1, sizeof(style_item)), initial, sizeof(style_item));
            token = strtok_r(0, " ", &space_tokenizer);
            float r = strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float g = strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float b = strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float a = token ? strtof(token, 0) : 1;
            style->fg_color = cf4(r, g, b, a);

          } else if (strcmp("line-height", token) == 0) {
            /* line-height [float] */

            token = strtok_r(0, " ", &space_tokenizer);
            float f = strtof(token, 0);
            if (defined_style) {
              defined_style->line_height = f;
            } else {
              style->line_height = f;
            }

          } else if (token[0] != command_starter) {
            /* unknown */

            error("unknown command: %s", token);
            token = strtok_r(0, " ", &space_tokenizer);
            while (token) {
              error("unknown attribute: %s", token);
              token = strtok_r(0, " ", &space_tokenizer);
            }
          }
          token = strtok_r(0, " ", &space_tokenizer);
        }

      } else { /* the rest of the text of a slide */

        if (!slide) {
          error("no slide before -- %s", line);
          continue;
        }

        item = Calloc(1, sizeof(text_item));
        push(slide->items, item);
        item->type = text_slide;
        item->y = yyy == MINFLOAT ? initialY : yyy;
        size_t len = strlen(line);
        item->text = malloc(len * sizeof(char) + 1);
        memcpy(item->text, line, len + 1);
        push(slide->styles, style ? : initial);
      }

      /* advance to the next line */
      line = strtok_r(0, "\n", &line_tokenizer);
    }
  }

  /* the previous index was saved; ensure slide index still ok */
  the_show->index = the_show->index < 0 ? 0
      : min(the_show->index, count(the_show->slides) - 1);
  return the_show;
}

slide_show *default_show() {
  slide_show *the_show = Calloc(1, sizeof(slide_show));

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

    slide = Calloc(1, sizeof(slide_item));
    push(the_show->slides, slide);

    slide->bg_color = cf4(0, 0, 0, .8);

    for (int j = 0; j < count(slides[i]); ++j) {
      char *str = slides[i][j];

      style = Calloc(1, sizeof(style_item));
      push(slide->styles, style);

      style->style = (i + j) % num_styles;
      style->family = (i + j) % num_families;
      style->align = (i + j) % num_alignments;
      style->size = .1f;
      style->fg_color = cf4(1, 1, 1, 1);

      text_item *item = Calloc(1, sizeof(text_item));
      push(slide->items, item);

      item->type = text_slide;
      item->text = str;
      item->y = .5;
    }
  }
  return the_show;
}

void render_slide(SDL_Renderer *renderer, int w, int h,
                  slide_show *show, font *fonts) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  slide_item *current_slide = show->slides[show->index];
  SDL_Color slide_bg = current_slide->bg_color;
  SDL_SetRenderDrawColor(renderer,
                         slide_bg.r, slide_bg.g, slide_bg.b, slide_bg.a);
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
    TTF_Font *f;
    if ((f = TTF_OpenFont(
        find_font(fonts, font_idx)->filename,
        (int) (style->size * h)))) {
      SDL_Texture *shadow =
          texturize_text(renderer, f,
                         item->text,
                         cf4(.1, .1, .1, .1),
                         &box, SDL_BLENDMODE_ADD);
      SDL_Texture *slide_text =
          texturize_text(renderer, f,
                         item->text,
                         style->fg_color,
                         &box, SDL_BLENDMODE_BLEND);
      TTF_CloseFont(f);
      box.y -= -(box.h * 2 / 3 + slide_i * box.h * style->line_height);
      switch (style->align) {
        default:
        case left:
          box.x += (int) (w * style->margins_x);
          break;
        case center:
          box.x += w / 2 - box.w / 2;
          break;
        case right:
          box.x = w - box.w - (int) (w * style->margins_x);
          break;
      }
      SDL_Rect rect = box;
      rect.x++;
      rect.y++;
      SDL_RenderCopy(renderer, shadow, 0, &rect);
      SDL_RenderCopy(renderer, slide_text, 0, &box);
      SDL_DestroyTexture(shadow);
      SDL_DestroyTexture(slide_text);
    } else {
      assert(!"needs a font");
    }
  }
}

SDL_Texture *
texturize_text(SDL_Renderer *renderer, TTF_Font *font, char *string,
               SDL_Color fg, SDL_Rect *r, SDL_BlendMode mode) {
  SDL_Surface *t;
  if (!(t = TTF_RenderText_Blended(font, string, fg))) return 0;
  assert(!SDL_SetSurfaceBlendMode(t, mode));
  SDL_Texture *words = SDL_CreateTextureFromSurface(renderer, t);
  SDL_FreeSurface(t);
  SDL_QueryTexture(words, 0, 0, &r->w, &r->h);
  assert(!SDL_SetTextureBlendMode(words, mode));
  return words;
}

font *find_font(font *fonts, char *name) {
  font *s;
  HASH_FIND_STR(fonts, name, s);
  return s;
}
