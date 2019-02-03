//
// Created by justin on 1/12/19.
//

#include "show.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCDFAInspection"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF

#include "../src/lib/stb_image.h"

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

void free_styles(style_item **saved_styles) {
  style_item *some_style, *temp;
  HASH_ITER(hh, *saved_styles, some_style, temp) {
    HASH_DEL(*saved_styles, some_style);
    free(some_style);
  }
}

int free_show(slide_show *show, style_item **saved_styles) {
  if (!show) return 0;
  for (int i = 0; i < count(show->slides); ++i) {
    slide_item *slide = show->slides[i];
    free(slide->title);
    // slide->grocery_items;
    // slide->using;
    // slide->title;
    for (int j = 0; j < count(slide->styles); ++j) {
//      free(slide->styles[j]);
//      free(slide->styles[j]->name);
//      slide->styles[j]->name = 0;
//      free(slide->styles[j]);
    }
    free(slide);
  }
  free_styles(saved_styles);
  for (int j = 0; j < count(show->positions); ++j)
    free(show->positions[j]);

  stretch_free(show->positions);
  stretch_free(show->slides);
  free(show);
}

slide_show *init_slides(int idx, style_item **saved_styles,
                        char *content) {
  slide_show *the_show = 0;

  if (!content) return default_show();

  /* make a new show from the content */
  int command_starter = '.';
  int slide_starter = '#';
  int comment_starter = ';';

  the_show = Calloc(1, sizeof(slide_show));
  if (idx > 0) the_show->index = idx;

  slide_item *slide = 0;
  style_item *style = 0;
  style_item *style_declaration = 0;
  bool reassign_y = false;
  float new_y = .0f;

  SDL_Color bg = cf4(1, 1, 1, 1); // default slide bg
  SDL_Color color;

  char *line_tokenizer, *space_tokenizer;
  char *line = strtok_r(content, "\n", &line_tokenizer);
  while (line) {
    debug("(line ): %s", line);

    if (line[0] == comment_starter) {
      /* ; comment line */

    } else if (line[0] == slide_starter) {
      /* # start of a slide */

      // boxes always start at the top on a new slide
      reassign_y = true;
      new_y = .0f;

      // eats a space before the actual title
      strtok_r(line, " ", &space_tokenizer);
      slide = Calloc(1, sizeof(slide_item));
      char *title = strtok_r(0, "\n", &space_tokenizer);
      slide->title = title ? strcpy(Malloc(strlen(title) + 1), title)
          : strcpy(Malloc(5), "none");
      slide->bg_color = bg;
      push(the_show->slides, slide);
      info("reassigned slide pointer -> '%s'", slide->title);

    } else if (line[0] == command_starter) {
      /* . [*]... start of command */

      // Note: Warning!!! please always re-assign token!
      //  this will avoid errors by always walking the data!
      char *token = strtok_r(line, " ", &space_tokenizer);
      while (token) {

        if (strcmp("font", token) == 0) {
          /* . font [float] [*]... */

          token = strtok_r(0, " ", &space_tokenizer);
          style = style_declaration ? :
              memcpy(Calloc(1, sizeof(style_item)),
                     style ? : &DEFAULT_STYLE, sizeof(style_item));
          style->size = !token ? .1f : strtof(token, 0);
          while (token) {
            set_fam(style, token);
            set_style(style, token);
            set_align(style, token);
            token = strtok_r(0, " ", &space_tokenizer);
          }
          info("assigned font props to style '%s'",
               !style ? "unknown style" : style->name);

        } else if (strcmp("#", token) == 0) {
          /* . # template slide */

          token = strtok_r(0, "\n", &space_tokenizer);

          template_slide *template;
          HASH_FIND_STR(the_show->template_slides, token, template);
          if (!template) {
            template = Calloc(1, sizeof(template_slide));
            size_t len = strlen(token);
            template->id = strcpy(Malloc(len * sizeof(char) + 1), token);
            HASH_ADD_STR(the_show->template_slides, id, template);

            slide = template->slide = !template ? 0
                : Calloc(1, sizeof(slide_item));
            info(GREEN
                     "start saving attributes to slide '%s' at %p",
                 token, template->slide);
            slide->title = strcpy(Malloc(len * sizeof(char) + 1), token);
          } else {
            slide = 0;
            info(RED
                     "attempt to redefine slide template '%s'!"
                     RESET, token);
          }

        } else if (strcmp("using", token) == 0) {

          token = strtok_r(0, "\n", &space_tokenizer);
          template_slide *template;
          HASH_FIND_STR(the_show->template_slides, token, template);
          if (template) {
            info(BLUE
                     "found slide template: '%s' at %p"
                     RESET,
                 token, template->slide);
            push(slide->using, template->slide);
          } else {
            info(RED
                     "attempt to use a slide template"
                     " that doesn't exist '%s'"
                     RESET, token);
          }

        } else if (strcmp("define-style", token) == 0) {
          /* . define-style [unique-name] */

          token = strtok_r(0, "\n", &space_tokenizer);
          style_item *found = 0;
          HASH_FIND_STR(*saved_styles, token, found);
          if (!found) {
            style_declaration = memcpy(
                Calloc(1, sizeof(style_item)),
                style ? : &DEFAULT_STYLE, sizeof(style_item));
            size_t len = strlen(token);
            char *style_name = strcpy(Malloc(len * sizeof(char) + 1), token);
            style_declaration->name = style_name;
            HASH_ADD_STR(*saved_styles, name, style_declaration);
            info(GREEN
                     "start style prop definitions for '%s'", style_name);
          } else {
            style = found;
          }

        } else if (strcmp("save-style", token) == 0) {
          /* . save-style */

          if (style_declaration) {
            style_item *found = 0;
            HASH_FIND_STR(*saved_styles, style_declaration->name, found);
            if (found) {
              style = style_declaration;
              info(BLUE
                       "done! defined style '%s'"
                       RESET, found->name);
            } else {
              info(RED
                       "oh no! expected to find style '%s'!"
                       RESET, style->name);
            }
            style_declaration = 0;
          } else {
            info(RED
                     "tried to save a style before defining one"
                     RESET);
          }

        } else if (strcmp("style", token) == 0) {
          /* . style [name] */

          token = strtok_r(0, "\n", &space_tokenizer);
          style_item *found = 0;
          HASH_FIND_STR(*saved_styles, token, found);
          if (found) {
            style = found;
            info(MAGENTA
                     "found style to reuse: '%s'"
                     RESET, found->name);
          } else {
            info(RED
                     "can't find style named %s"
                     RESET, token);
          }

        } else if (strcmp("y", token) == 0) {
          /* . y [float] */

          token = strtok_r(0, " ", &space_tokenizer);
          new_y = !token ? 0 : strtof(token, 0);
          reassign_y = true;

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
            info(RED
                     "No slide yet; define a slide before "
                     "setting the background color '%s'"
                     RESET, line);
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
          style = style_declaration ? :
              memcpy(Calloc(1, sizeof(style_item)),
                     style ? : &DEFAULT_STYLE, sizeof(style_item));
          style->fg_color = color;

        } else if (strcmp("line-height", token) == 0) {
          /* line-height [float] */

          token = strtok_r(0, " ", &space_tokenizer);
          style = style_declaration ? :
              memcpy(Calloc(1, sizeof(style_item)),
                     style ? : &DEFAULT_STYLE, sizeof(style_item));
          float f = !token ? : strtof(token, 0);
          style->line_height = f;

        } else if (token[0] != command_starter) {
          /* unknown */

          info(RED
                   "unknown command: %s"
                   RESET, token);
          token = strtok_r(0, " ", &space_tokenizer);
          while (token) {
            info(RED
                     "unknown attribute: %s"
                     RESET, token);
            token = strtok_r(0, " ", &space_tokenizer);
          }
        }

        /* advance to next space-separated token */
        token = strtok_r(0, " ", &space_tokenizer);
      }
    } else {
      /* plain text */

      if (slide) {
        item_grocer *item = Calloc(1, sizeof(item_grocer));
        item->type = text_t_item;
        push(slide->grocery_items, item);
        style = style ? : memcpy(Calloc(1, sizeof(style_item)),
                                 &DEFAULT_STYLE, sizeof(style_item));
        push(slide->styles, style);
        size_t len = strlen(line);
        item->item.text = memcpy(Malloc(len * sizeof(char) + 1),
                                 line, len + 1);
        if (reassign_y) {
          // either we saw a . y command or
          // we are in a new slide. in either case
          // the new_y is set to the correct y coord
          item->pos = Calloc(1, sizeof(point));
          push(the_show->positions, item->pos);
          item->pos->y = new_y;
          item->pos->x = 777;
          reassign_y = false;
        } else {
          // interpret 0 as being the use case for continuing
          // to the next line automatically changing y by
          // the line-height
          item->pos = 0;
        }

        info("pushed some text onto slide '%s': %s", slide->title, line);
      } else {
        info(RED
                 "Error: plain text before `# slide heading`;"
                 " define a slide before '%s'"
                 RESET, line);
      }
    }

    /* advance to the next line */
    line = strtok_r(0, "\n", &line_tokenizer);
  }

  for (int i = 0; i < count(the_show->slides); ++i) {
    assert(count(the_show->slides[i]->styles) ==
           count(the_show->slides[i]->grocery_items));
  }
  /* the previous index was saved; ensure slide index still ok */
  the_show->index = the_show->index < 0 ? 0
      : min(the_show->index, count(the_show->slides) - 1);
  return the_show;
}

void use(void) {
  item_grocer **grocery_items = 0;
  item_grocer *item = Calloc(1, sizeof(item_grocer));

  item->type = text_t_item;
  char *s = "i'm a little teapot short and stout";
  item->item.text = strcpy(Malloc(sizeof(strlen(s)) + 1), s);

  push(grocery_items, item);

  switch (item->type) {
    case text_t_item: {
      info("draw text %s", item->item.text);
      break;
    }
    case image_t_item: {
      info("draw image %p", item->item.image);
      break;
    }
  }
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

      item_grocer *item = Calloc(1, sizeof(item_grocer));
      push(slide->grocery_items, item);

      item->item.text = str;
    }
  }
  return the_show;
}

void render_slide(SDL_Renderer *renderer, int w, int h,
                  slide_show *show, font *fonts) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  slide_item *current_slide = show->slides[show->index];
  SDL_Color slide_bg = current_slide->bg_color;
  SDL_SetRenderDrawColor(
      renderer, slide_bg.r, slide_bg.g, slide_bg.b, slide_bg.a);
  SDL_RenderFillRect(renderer, 0);

  SDL_Rect x = {100, 100, 100, 100};
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
  SDL_RenderFillRect(renderer, &x);

  for (int i = 0; i < count(current_slide->using); ++i) {
    slide_item *using = current_slide->using[i];
    if (using) draw_slide_items(renderer, w, h, fonts, using);
  }
  draw_slide_items(renderer, w, h, fonts, current_slide);
}

void draw_slide_items(const SDL_Renderer *renderer, int w, int h,
                      const font *fonts, const slide_item *current_slide) {
  point top_left = {0, 0};
  int line_number = 0;
  point *last_box = 0;
  SDL_Rect rect = {0};
  point box;

  for (int i = 0; i < count(current_slide->grocery_items); ++i) {
    item_grocer *item = current_slide->grocery_items[i];
    style_item *style = current_slide->styles[i];

    // if item pos, initialize our box there
    // when no item pos, we should use the last box
    // when no last box, we should use the top left
    box = item->pos ? *item->pos : last_box ? *last_box : top_left;
    // seeing a new pos resets the y-coordinate
    // and therefore our number line is also reset
    // so our text flows
    if (item->pos) line_number = 0;

    rect.y = (int) (h * box.y);
    rect.x = 0;

    char *fam = get_fam(style);
    char *sty = get_style(style);
    char font_idx[strlen(fam) + strlen(sty) + 1];
    strcpy(font_idx, fam);
    strcat(font_idx, sty);
    // todo using sdl and sdl_ttf source would allow us
    //  to call the SetFontSize() instead of reading the
    //  thing every frame. that way we can keep the pointers
    //  and don't have to store all the sizes. we can just
    //  keep the pointers and set the size in the render call
    TTF_Font *f;
    if ((f = TTF_OpenFont(
        find_font(fonts, font_idx)->filename,
        (int) (style->size * h)))) {
      SDL_Texture *shadow_text =
          texturize_text(renderer, f,
                         item->item.text,
                         cf4(0, 0, 0, .4),
                         &rect, SDL_BLENDMODE_BLEND);
      SDL_Texture *slide_text =
          texturize_text(renderer, f,
                         item->item.text,
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
      SDL_RenderCopy(renderer, shadow_text, 0, &rect);
      rect.x -= SHADOW_DISTANCE;
      rect.y -= SHADOW_DISTANCE;
      SDL_RenderCopy(renderer, slide_text, 0, &rect);
      SDL_DestroyTexture(shadow_text);
      SDL_DestroyTexture(slide_text);
    } else {
      assert(!"needs a better font failure mechanism");
    }
    last_box = &box;
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

  SDL_SetTextureAlphaMod(words, fg.a);

  return words;
}

SDL_Texture *get_texture_from_image(SDL_Renderer *, char *);

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

SDL_Cursor *get_cursor();

SDL_Cursor *get_cursor() {

  SDL_Cursor *cursor = 0;

  /*SDL_Cursor **/ // cursor = get_cursor();
  SDL_FreeCursor(cursor);

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

font *find_font(font *fonts, char *name) {
  font *s;
  HASH_FIND_STR(fonts, name, s);
  return s;
}

#pragma clang diagnostic pop