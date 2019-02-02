//
// Created by justin on 1/12/19.
//

#include <values.h>
#include <assert.h>
#include "show.h"
#include "lib/stretchy.h"
#include "lib/csapp.h"

#pragma clang diagnostic push
/* clion, you're fucking retarded */
#pragma clang diagnostic ignored "-Wunknown-pragmas"

style_hash *saved_styles;

#pragma clang diagnostic push
/* AGAIN, clion, you're fucking retarded */
#pragma ide diagnostic ignored "OCDFAInspection"

void set_fam(style_item *style, const char *token) {
  for (int i = 0; i < num_families; ++i)
    if (strcmp(family[i].name, token) == 0) {
      style->family = family[i].f;
      break;
    }
}

#pragma clang diagnostic pop

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

static style_item DEFAULT_STYLE = {
    /* size        float       */  .1,
    /* style       font_style  */  italic,
    /* family      font_family */  serif,
    /* align       align_text  */  center,
    /* line_height float       */  1.f,
    /* margins_x   float       */  0.05f,
    /* fg_color    SDL_Color   */  {0, 0, 0, 255},
    /* name        char        */  "default",
};

template_slide *templatize(char *name, template_slide **template_slides) {
  template_slide *template;
  HASH_FIND_STR(*template_slides, name, template);
  if (template) return template;
  template = Calloc(1, sizeof(template_slide));
  template->id = name;
  HASH_ADD_STR(*template_slides, id, template);
  return template;
}

slide_show *init_slides(slide_show *previous_show, char *content) {
  slide_show *the_show = 0;

  if (!content) return default_show();

  /* make a new show from the content */
  int command_starter = '.';
  int slide_starter = '#';
  int comment_starter = ';';

  the_show = Calloc(1, sizeof(slide_show));
  if (previous_show) the_show->index = previous_show->index;

  slide_item *slide = 0;
  style_item *style = 0;
  text_item *item = 0;
  point *box = 0;
  SDL_Color bg = cf4(0, 0, 0, .9);
  SDL_Color color;

  char *line_tokenizer, *space_tokenizer;
  char *line = strtok_r(content, "\n", &line_tokenizer);
  while (line) {
    debug("(line ): %s", line);

    if (line[0] == comment_starter) {
      /* ; comment line */

    } else if (line[0] == slide_starter) {
      /* # start of a slide */

      box = 0; // note boxes always start at the top on a new slide

      // eats a space before the actual title
      strtok_r(line, " ", &space_tokenizer);
      slide = Calloc(1, sizeof(slide_item));
      slide->title = strtok_r(0, "\n", &space_tokenizer) ? :
          strcpy(Malloc(5), "none");
      slide->bg_color = bg;
      push(the_show->slides, slide);
      info("reassigned slide pointer -> '%s'", slide->title);

    } else if (line[0] == command_starter) {
      /* . [*]... start of command */

      char *token = strtok_r(line, " ", &space_tokenizer);
      while (token) {
        if (strcmp("font", token) == 0) {
          /* . font [float] [*]... */

          token = strtok_r(0, " ", &space_tokenizer);
          style = memcpy(Calloc(1, sizeof(style_item)),
                         style ? : &DEFAULT_STYLE, sizeof(style_item));
          style->size = !token ? .1f : strtof(token, 0);
          while (token) {
            set_fam(style, token);
            set_style(style, token);
            set_align(style, token);
            token = strtok_r(0, " ", &space_tokenizer);
          }
          info("assigned font to '%s'", !slide ? "a style i guess" : slide->title);

        } else if (strcmp("#", token) == 0) {
          /* . # template slide */

          char *slide_title = strtok_r(0, "\n", &space_tokenizer);
          template_slide *x = templatize(slide_title, &the_show->template_slides);
          slide = x->slide = !x ? 0 : Calloc(1, sizeof(slide_item));
          info(GREEN
                   "start saving attributes to slide '%s' at %p", slide_title, x->slide);
          slide->title = slide_title;

        } else if (strcmp("using", token) == 0) {

          char *slide_title = strtok_r(0, "\n", &space_tokenizer);
          template_slide *x = templatize(slide_title, &the_show->template_slides);
          info(BLUE
                   "found slide template: '%s' at %p"
                   RESET, slide_title, x->slide);
          push(slide->using, x->slide);

        } else if (strcmp("define-style", token) == 0) {
          /* . define-style [unique-name] */

          token = strtok_r(0, " ", &space_tokenizer);
          style_hash *found = 0;
          HASH_FIND_STR(saved_styles, token, found);
          if (!found) {
            style = memcpy(Calloc(1, sizeof(style_item)),
                           style ? : &DEFAULT_STYLE, sizeof(style_item));
            style->name = token;
          } else {
            style = found->style;
          }

        } else if (strcmp("save-style", token) == 0) {
          /* . save-style */

          if (style) {
            style_hash *found = 0;
            found = Malloc(sizeof(style_hash));
            found->name = style->name;
            found->style = style;
            HASH_ADD_STR(saved_styles, name, found);
          } else {
            error("tried to save a style before defining one");
          }

        } else if (strcmp("style", token) == 0) {
          /* . style [name] */

          token = strtok_r(0, " ", &space_tokenizer);
          style_hash *found = 0;
          HASH_FIND_STR(saved_styles, token, found);
          if (found) style = found->style;
          else
            error("can't find style named %s", token);

        } else if (strcmp("y", token) == 0) {
          /* . y [float] */

          token = strtok_r(0, " ", &space_tokenizer);
          box = Calloc(1, sizeof(SDL_Rect));
          box->y = !token ? 0 : strtof(token, 0);

        } else if (strcmp("bg", token) == 0) {
          /* . bg [float_r] [float_g] [float_b] [? alpha] */

          if (slide) {
            token = strtok_r(0, " ", &space_tokenizer);
            float r = !token ? : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float g = !token ? : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float b = !token ? : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            float a = token ? strtof(token, 0) : 1;
            bg = cf4(r, g, b, a);
            slide->bg_color = bg;
          } else {
            error("No slide yet; define a slide before "
                  "setting the background color '%s'", line);
          }

        } else if (strcmp("color", token) == 0) {
          /* text color [float_r] [float_g] [float_b] */

          token = strtok_r(0, " ", &space_tokenizer);
          float r = !token ? : strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float g = !token ? : strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float b = !token ? : strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float a = token ? strtof(token, 0) : 1;
          color = cf4(r, g, b, a);
          style = memcpy(Calloc(1, sizeof(style_item)),
                         style ? : &DEFAULT_STYLE, sizeof(style_item));
          style->fg_color = color;

        } else if (strcmp("line-height", token) == 0) {
          /* line-height [float] */

          token = strtok_r(0, " ", &space_tokenizer);
          style = memcpy(Calloc(1, sizeof(style_item)),
                         style ? : &DEFAULT_STYLE, sizeof(style_item));
          float f = !token ? : strtof(token, 0);
          style->line_height = f;

        } else if (token[0] != command_starter) {
          /* unknown */

          error("unknown command: %s", token);
          token = strtok_r(0, " ", &space_tokenizer);
          while (token) {
            error("unknown attribute: %s", token);
            token = strtok_r(0, " ", &space_tokenizer);
          }
        }

        /* advance to next space-separated token */
        token = strtok_r(0, " ", &space_tokenizer);
      }
    } else {
      /* plain text */

      if (slide) {
        item = Calloc(1, sizeof(text_item));
        push(slide->items, item);
        style = style ? : memcpy(Calloc(1, sizeof(style_item)),
                                 &DEFAULT_STYLE, sizeof(style_item));
        push(slide->styles, style);
        size_t len = strlen(line);
        item->text = memcpy(Malloc(len * sizeof(char) + 1),
                            line, len + 1);
        box = box ? : Calloc(1, sizeof(SDL_Rect));
        push(slide->points, box);

        info("pushed some text onto slide '%s': %s", slide->title, line);
      } else {
        error("Error: plain text before `# slide heading`;"
              " define a slide before '%s'", line);
      }
    }

    /* advance to the next line */
    line = strtok_r(0, "\n", &line_tokenizer);
  }

  for (int i = 0; i < count(the_show->slides); ++i) {
    assert(count(the_show->slides[i]->points) == count(the_show->slides[i]->styles));
    assert(count(the_show->slides[i]->points) == count(the_show->slides[i]->items));
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

      item->text = str;
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

  for (int i = 0; i < count(current_slide->using); ++i) {
    slide_item *using = current_slide->using[i];
    if (using) draw_slide_items(renderer, w, h, fonts, using);
  }

  draw_slide_items(renderer, w, h, fonts, current_slide);
}

void draw_slide_items(const SDL_Renderer *renderer, int w, int h,
                      const font *fonts, const slide_item *current_slide) {
  int line_number = 0;
  point *last_box = 0;

  for (int i = 0; i < count(current_slide->items); ++i) {
    text_item *item = current_slide->items[i];
    style_item *style = current_slide->styles[i];
    point *box = current_slide->points[i];

    // boxes reset the y-coordinate
    if (box != last_box) line_number = 0;

    SDL_Rect rect = {0, (int) (h * box->y), 0, 0};

    char *fam = get_fam(style);
    char *sty = get_style(style);
    char font_idx[strlen(fam) + strlen(sty) + 1];
    strcpy(font_idx, fam);
    strcat(font_idx, sty);
    TTF_Font *f;
    if ((f = TTF_OpenFont(
        find_font(fonts, font_idx)->filename,
        (int) (style->size * h)))) {
      SDL_Texture *slide_text =
          texturize_text(renderer, f,
                         item->text,
                         style->fg_color,
                         &rect, SDL_BLENDMODE_BLEND);
      TTF_CloseFont(f);
      int vertical_align = 0; // todo
      rect.y = rect.y + vertical_align +
               (int) (line_number++ * rect.h * style->line_height);
      switch (style->align) {
        default:
        case left:
          rect.x += (int) (w * style->margins_x);
          break;
        case center:
          rect.x += w / 2 - rect.w / 2;
          break;
        case right:
          rect.x = w - rect.w - (int) (w * style->margins_x);
          break;
      }
      rect.x++;
      rect.y++;

      SDL_RenderCopy(renderer, slide_text, 0, &rect);
      SDL_DestroyTexture(slide_text);

      last_box = box;
    } else {
      assert(!"needs a better font failure mechanism");
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

#pragma clang diagnostic pop