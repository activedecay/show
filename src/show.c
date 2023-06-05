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

char *RES_DIR = "./res/";

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
  /* size         float       */ .1f,
  /* style        font_style  */ italic,
  /* family       font_family */ serif,
  /* align        align_text  */ center,
  /* line_height  float       */ 1.f,
  /* margins_x    float       */ 0.05f,
  /* fg_color     SDL_Color   */ {0, 0, 0, 255},
  /* shadow_color SDL_Color   */ {0, 0, 0, 102},
  /* name         char        */ "default",
};

void free_styles(style_item **saved_styles) {
  style_item *some_style, *temp;
  HASH_ITER(hh, *saved_styles, some_style, temp) {
    HASH_DEL(*saved_styles, some_style);
    free(some_style);
  }
}

void free_images(image_hash *images) {
  image_hash *img, *temp;
  HASH_ITER(hh, images, img, temp) {
    HASH_DEL(images, img);
    // note NEVER DO THIS ON ANY THREAD OTHER THAN MAIN
    //SDL_DestroyTexture(img->image); // you royally screwed up
    info(GREEN"freeing image '%s'@'%s'"RESET, img->id, img->image_res_name);
    free(img->id);
    free(img->image_res_name);
    free(img);
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
    free(slide);
  }
  free_styles(saved_styles);
  free_images(show->images);
  for (int j = 0; j < count(show->positions); ++j)
    free(show->positions[j]);

  stretch_free(show->styles);
  stretch_free(show->positions);
  stretch_free(show->slides);
  free(show);
}

int
filter_image_files(const struct dirent *d) {
  char *name = d->d_name;
  size_t len = strlen(name);

  return
    strncmp(&name[len - 4], ".png", 4) == 0 ||
    strncmp(&name[len - 4], ".bmp", 4) == 0 ||
    strncmp(&name[len - 4], ".jpg", 4) == 0 ||
    strncmp(&name[len - 4], ".gif", 4) == 0;
}

/** returns pointer to malloc'd string of the file path, or else 0 */
char *
find_resource(char *resource_name) {
  struct dirent **namelist = 0;
  int n = scandir(RES_DIR, &namelist, filter_image_files, 0);
  char *retval = 0;
  while (n--) {
    if (!retval && strstr(namelist[n]->d_name, resource_name) != 0) {
      char temp[(strlen(RES_DIR) + strlen(namelist[n]->d_name) * sizeof(char)) + 1];
      strcpy(temp, RES_DIR);
      strcat(temp, namelist[n]->d_name);
      retval = strcpy(malloc(strlen(temp) * /* destination */
                             sizeof(char) + 1),
                      temp); /* source */
    }
    free(namelist[n]);
  }
  free(namelist);
  return retval;
}

slide_show *init_slides(int idx, style_item **saved_styles, char *content) {
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

  SDL_Color bg = cf4(1, 1, 1, 1);
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

      char *title = strtok_r(&line[1], "\n", &space_tokenizer);
      slide = Calloc(1, sizeof(slide_item));
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
          style = style_declaration ?:
                  memcpy(Calloc(1, sizeof(style_item)),
                         style ?: &DEFAULT_STYLE, sizeof(style_item));
          style->size = !token ? .1f : strtof(token, 0);
          while (token) {
            set_fam(style, token);
            set_style(style, token);
            set_align(style, token);
            token = strtok_r(0, " ", &space_tokenizer);
          }
          info("assigned font props to style '%s'", !style ? "unknown style" : style->name);

        } else if (strcmp("define-image", token) == 0) {
          /* . define-image [alias] [filename] */

          token = strtok_r(0, " ", &space_tokenizer);
          char *image_alias = token;
          token = strtok_r(0, "\n", &space_tokenizer);
          char *filename = token;
          char *res_name = find_resource(filename);
          image_hash *found;
          HASH_FIND_STR(the_show->images, image_alias, found);
          if (found) {
            info(RED "there's already an image named '%s'" RESET, image_alias);
          } else {
            image_hash *ihash = Calloc(1, sizeof(image_hash));
            ihash->image_res_name = res_name;
            size_t len = strlen(image_alias);
            ihash->id = strcpy(Malloc(len * sizeof(char) + 1),
                               image_alias);
            HASH_ADD_STR(the_show->images, id, ihash);
            info("defined image '%s' from shortname '%s'"
                 " in folder '%s' full path was '%s'",
                 image_alias, filename, RES_DIR, res_name);
          }
          //free(res_name);

        } else if (strcmp("image", token) == 0) {
          /* . image [*]... */

          token = strtok_r(0, " ", &space_tokenizer);
          info(YELLOW "using image '%s'" RESET, token);
          image_hash *found;
          HASH_FIND_STR(the_show->images, token, found);
          if (found) {
            item_grocer *item = Calloc(1, sizeof(item_grocer));
            item->type = image_t_item;
            item->item.text = found->image_res_name;
            item->src_rect = (rect) {0, 0, 1, 1};
            item->dst_rect = (rect) {0, 0, 1, 1};
            token = strtok_r(0, " ", &space_tokenizer);
            item->src_rect.x = !token ? .0f : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            item->src_rect.y = !token ? .0f : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            item->src_rect.w = !token ? 1.f : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            item->src_rect.h = !token ? 1.f : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            item->dst_rect.x = !token ? .0f : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            item->dst_rect.y = !token ? .0f : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            item->dst_rect.w = !token ? 1.f : strtof(token, 0);
            token = strtok_r(0, " ", &space_tokenizer);
            item->dst_rect.h = !token ? 1.f : strtof(token, 0);
            push(slide->grocery_items, item);
          } else {
            info(RED "can't find image named '%s'" RESET, token);
          }

        } else if (strcmp("#", token) == 0) {
          /* . # template slide */

          token = strtok_r(0, "\n", &space_tokenizer);

          template_slide *template;
          HASH_FIND_PTR(the_show->template_slides, token, template);
          if (!template) {
            template = Calloc(1, sizeof(template_slide));
            size_t len = strlen(token);
            template->id = strcpy(Malloc(len * sizeof(char) + 1), token);
            HASH_ADD_KEYPTR(hh, the_show->template_slides,
                            template->id, len, template);

            slide = template->slide = !template ? 0
                                                : Calloc(1, sizeof(slide_item));
            info(GREEN "start saving attributes to slide '%s' at %p", token, template->slide);
            slide->title = strcpy(Malloc(len * sizeof(char) + 1), token);
          } else {
            slide = 0;
            info(RED "attempt to redefine slide template '%s'!" RESET, token);
          }

        } else if (strcmp("using", token) == 0) {

          token = strtok_r(0, "\n", &space_tokenizer);
          template_slide *template;
          HASH_FIND_STR(the_show->template_slides, token, template);
          if (template) {
            info(BLUE "found slide template: '%s' at %p"
                   RESET, token, template->slide);
            push(slide->using, template->slide);
          } else {
            info(RED "attempt to use a slide template"
                     " that doesn't exist '%s'" RESET, token);
          }

        } else if (strcmp("define-style", token) == 0) {
          /* . define-style [unique-name] */

          token = strtok_r(0, "\n", &space_tokenizer);
          style_item *found = 0;
          HASH_FIND_STR(*saved_styles, token, found);
          if (!found) {
            style_declaration = memcpy(
              Calloc(1, sizeof(style_item)),
              style ?: &DEFAULT_STYLE, sizeof(style_item));
            size_t len = strlen(token);
            char *style_name = strcpy(Malloc(len * sizeof(char) + 1), token);
            style_declaration->name = style_name;
            HASH_ADD_STR(*saved_styles, name, style_declaration);
            info(GREEN "start style prop definitions for '%s'", style_name);
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
              info(BLUE "done! defined style '%s'" RESET, found->name);
            } else {
              info(RED "oh no! expected to find style '%s'!" RESET, style->name);
            }
            style_declaration = 0;
          } else {
            info(RED "tried to save a style before defining one" RESET);
          }

        } else if (strcmp("style", token) == 0) {
          /* . style [name] */

          token = strtok_r(0, "\n", &space_tokenizer);
          style_item *found = 0;
          HASH_FIND_STR(*saved_styles, token, found);
          if (found) {
            style = found;
            info(MAGENTA "found style to reuse: '%s'" RESET, found->name);
          } else {
            info(RED "can't find style named %s" RESET, token);
          }

        } else if (strcmp("y", token) == 0) {
          /* . y [float] */

          token = strtok_r(0, " ", &space_tokenizer);
          new_y = !token ? 0 : strtof(token, 0); // todo overflow, underflow, non-parsed
          reassign_y = true;

        } else if (strcmp("bg", token) == 0) {
          /* . bg [float_r] [float_g] [float_b] [? alpha] */

          token = strtok_r(0, " ", &space_tokenizer);
          float r = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float g = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float b = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float a = token ? strtof(token, 0) : 1;
          bg = cf4(r, g, b, a);

        } else if (strcmp("color", token) == 0) {
          /* . color [float_r] [float_g] [float_b] [float_a] */

          token = strtok_r(0, " ", &space_tokenizer);
          float r = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float g = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float b = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float a = token ? strtof(token, 0) : 1;
          color = cf4(r, g, b, a);
          style = style_declaration ?:
                  memcpy(Calloc(1, sizeof(style_item)),
                         style ?: &DEFAULT_STYLE, sizeof(style_item));
          style->fg_color = color;

        } else if (strcmp("shadow", token) == 0) {
          /* . shadow [float_r] [float_g] [float_b] */

          token = strtok_r(0, " ", &space_tokenizer);
          float r = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float g = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float b = !token ?: strtof(token, 0);
          token = strtok_r(0, " ", &space_tokenizer);
          float a = token ? strtof(token, 0) : 1;
          color = cf4(r, g, b, a);
          style = style_declaration ?:
                  memcpy(Calloc(1, sizeof(style_item)),
                         style ?: &DEFAULT_STYLE, sizeof(style_item));
          style->shadow_color = color;

        } else if (strcmp("line-height", token) == 0) {
          /* . line-height [float] */

          token = strtok_r(0, " ", &space_tokenizer);
          style = style_declaration ?:
                  memcpy(Calloc(1, sizeof(style_item)),
                         style ?: &DEFAULT_STYLE, sizeof(style_item));
          float f = !token ?: strtof(token, 0);
          style->line_height = f;

        } else if (strcmp("margin", token) == 0) {
          /* . margin [float_vertical] [?float_horizontal] */

          info(GREEN "set margin: %s" RESET, space_tokenizer);
          token = strtok_r(0, " ", &space_tokenizer); // todo vertical and horizontal
          style = style_declaration ?:
                  memcpy(Calloc(1, sizeof(style_item)),
                         style ?: &DEFAULT_STYLE, sizeof(style_item));
          style->margins_x = !token ? 0 : strtof(token, 0);
        } else if (token[0] != command_starter) {
          /* unknown */

          info(RED "unknown command: %s" RESET, token);
          token = strtok_r(0, " ", &space_tokenizer);
          while (token) {
            info(RED "unknown attribute: %s" RESET, token);
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
        item->style = style = style ?: memcpy(Calloc(1, sizeof(style_item)),
                                              &DEFAULT_STYLE, sizeof(style_item));
        push(the_show->styles, style);
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
          item->pos->x = 77777; // todo unused 77777
          reassign_y = false;
        } else {
          // interpret 0 as being the use case for continuing
          // to the next line automatically changing y by
          // the line-height
          item->pos = 0;
        }

        info("pushed some text onto slide '%s': %s", slide->title, line);
      } else {
        info(RED "Error: plain text before `# slide heading`;"
                 " define a slide before '%s'" RESET, line);
      }
    }

    /* advance to the next line */
    line = strtok_r(0, "\n", &line_tokenizer);
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
      push(the_show->styles, style);

      style->style = (i + j) % num_styles;
      style->family = (i + j) % num_families;
      style->align = (i + j) % num_alignments;
      style->size = .1f;
      style->fg_color = cf4(1, 1, 1, 1);

      item_grocer *item = Calloc(1, sizeof(item_grocer));
      push(slide->grocery_items, item);
      item->style = style;
      item->item.text = str;
    }
  }
  return the_show;
}

void render_slide(SDL_Renderer *renderer, int w, int h,
                  slide_show *show, font *fonts, linkedlist *images) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  slide_item *current_slide = show->slides[show->index];
  SDL_Color slide_bg = current_slide->bg_color;
  SDL_SetRenderDrawColor(
    renderer, slide_bg.r, slide_bg.g, slide_bg.b, slide_bg.a);
  SDL_RenderFillRect(renderer, 0);

  /* generally, iterate over all slides;
   * draw images first, then text */

  for (int i = 0; i < count(current_slide->using); ++i) {
    slide_item *using = current_slide->using[i];
    if (using) draw_slide_items(renderer, w, h, fonts, using, images, image_t_item);
  }
  draw_slide_items(renderer, w, h, fonts, current_slide, images, image_t_item);

  for (int i = 0; i < count(current_slide->using); ++i) {
    slide_item *using = current_slide->using[i];
    if (using) draw_slide_items(renderer, w, h, fonts, using, images, text_t_item);
  }
  draw_slide_items(renderer, w, h, fonts, current_slide, images, text_t_item);
}


// todo (draw_slide_items)
//  using sdl and sdl_ttf source would allow us
//  to call the SetFontSize() instead of reading the
//  thing every frame. that way we can keep the pointers
//  and don't have to store all the sizes. we can just
//  keep the pointers and set the size in the render call.
//  implementing this feature would remove the OpenFont calls
//  (i guess) but my feeling is that you're full of shit
// historical justin made bolder statements than the today justin

/* iterate over the grocery list and draw each item on the renderer */
void draw_slide_items(SDL_Renderer *renderer, int width, int height,
                      const font *fonts, const slide_item *current_slide,
                      linkedlist *images, grocer_type filter) {
  SDL_Rect rect = {0}; /* origin's locations mapped to screen pixels (integers) */
  for (int i = 0; i < count(current_slide->grocery_items); ++i) {
    item_grocer *item = current_slide->grocery_items[i];
    if (item->type != filter) continue;

    switch (item->type) {
      case text_t_item: {

        style_item *style = item->style;
        int font_size = (int) (style->size * (float) height);

        if (item->pos) {
          rect.y = (int) (item->pos->y * (float) height);
        } // otherwise, don't update rect.y to keep the loop value increments
        rect.x = 0; // todo text x offset
        // draw text at rect
        // update rect.y += rect.h * style->line_height

        char *fam = get_fam(style);
        char *sty = get_style(style);
        char font_idx[strlen(fam) + strlen(sty) + 1];
        strcpy(font_idx, fam);
        strcat(font_idx, sty);

        TTF_Font *font;
        if ((font = TTF_OpenFont(find_font(fonts, font_idx)->filename, font_size))) {
          SDL_Texture *shadow_text =
            texturize_text(renderer, font, item->item.text,
                           style->shadow_color,
                           &rect, SDL_BLENDMODE_BLEND);
          SDL_Texture *slide_text =
            texturize_text(renderer, font, item->item.text,
                           style->fg_color,
                           &rect, SDL_BLENDMODE_BLEND);
          TTF_CloseFont(font);
          // todo text vertical align
          switch (style->align) {
            default:
            case left:
              rect.x += (int) ((float) width * style->margins_x);
              break;
            case center:
              rect.x += width / 2 - rect.w / 2;
              break;
            case right:
              rect.x = width - rect.w - (int) ((float) width * style->margins_x);
              break;
          }
          SDL_RenderCopy(renderer, shadow_text, 0, &rect);
          rect.x -= SHADOW_DISTANCE;
          rect.y -= SHADOW_DISTANCE;
          SDL_RenderCopy(renderer, slide_text, 0, &rect);
          SDL_DestroyTexture(shadow_text);
          SDL_DestroyTexture(slide_text);
          rect.y = rect.y + (int) ((float) rect.h * style->line_height);
        } else {
          assert(!"needs a better font failure mechanism");
        }
        break;
      }
      case image_t_item: {
        if (!item->image_texture) {

          bool found = false;
          image_item *ii;
          while (!found || images == 0) {
            ii = (image_item *) images->data;
            if (!ii) break;
            if (strcmp(ii->text, item->item.text) == 0) {
              found = true;
              item->item.image = ii->image;
              item->w = ii->width;
              item->h = ii->height;
            }
            images = images->next;
          }

          if (!found) {
            ii = Calloc(1, sizeof(image_item));
            images->data = ii;
            ii->text = Malloc(strlen(item->item.text) + 1);
            strcpy(ii->text, item->item.text);
            item->item.image = get_texture_from_image(renderer, item->item.text, ii);
            ii->image = item->item.image;
            item->w = ii->width;
            item->h = ii->height;
            images->next = Calloc(1, sizeof(linkedlist));
          } else {
            debug("renderer found an image: %s", ii->text);
          }

          item->image_texture = true;
        }
        SDL_Rect srcrect; // crop
        srcrect.x = item->w * item->src_rect.x;
        srcrect.y = item->h * item->src_rect.y;
        srcrect.w = item->w * item->src_rect.w;
        srcrect.h = item->h * item->src_rect.h;

        SDL_Rect dstrect; // scale
        dstrect.x = width * item->dst_rect.x;
        dstrect.y = height * item->dst_rect.y;
        dstrect.w = width * item->dst_rect.w;
        dstrect.h = height * item->dst_rect.h;

        SDL_RenderCopy(renderer, item->item.image, &srcrect, &dstrect);
        break;
      }
    }
  }
}

/* creates a texture using the ttf font, rendered in high definition.
 * queries the texture's width and height and stores the result in rect */
SDL_Texture *
texturize_text(SDL_Renderer *renderer, TTF_Font *font, char *string,
               SDL_Color fg, SDL_Rect *rect, SDL_BlendMode mode) {
  SDL_Surface *surf;
//  char tmp[4096];
//  sprintf(tmp, "%s %d, %d", string, r->w, r->h);
//  if (!(t = TTF_RenderText_Blended(font, tmp, fg))) return 0;
  if (!(surf = TTF_RenderText_Blended(font, string, fg))) return 0;
  assert(!SDL_SetSurfaceBlendMode(surf, mode));
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
  SDL_FreeSurface(surf);
  SDL_QueryTexture(texture, 0, 0, &rect->w, &rect->h);
  assert(!SDL_SetTextureBlendMode(texture, mode));

  SDL_SetTextureAlphaMod(texture, fg.a);

  return texture;
}

SDL_Texture *get_texture_from_image(SDL_Renderer *renderer, char *filename, image_item *ii) {
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
  ii->width = image_width;
  ii->height = image_height;
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

font *find_font(const font *fonts, char *name) {
  font *s;
  HASH_FIND_STR(fonts, name, s);
  return s;
}

#pragma clang diagnostic pop