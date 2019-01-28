//
// Created by justin on 1/12/19.
//

#include "show.h"
#include "lib/stretchy.h"

char *Sans = "/usr/share/fonts/truetype/freefont/FreeSans.ttf";
char *SansBold = "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf";
char *SansOblique = "/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf";
char *Serif = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf";
char *SerifBold = "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf";
char *SerifItalic = "/usr/share/fonts/truetype/freefont/FreeSerifItalic.ttf";
char *Mono = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";
char *MonoBold = "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf";
char *MonoOblique = "/usr/share/fonts/truetype/freefont/FreeMonoOblique.ttf";


slide_show *init_slides(char *content) {
  slide_show *the_show = 0;
  int command_starter = '.';
  int slide_starter = '#';

  if (content) {
    the_show = calloc(1, sizeof(slide_show));
    slide_item *slide = 0;
    style_item *style = 0;
    text_item *item = 0;

    style_item *initial;
    initial = calloc(1, sizeof(style_item));
    initial->style =  normal;
    initial->family = sans;
    initial->align =  center;
    initial->size =   .1f;

    char *line_tokenizer, *space_tokenizer;
    char *line = strtok_r(content, "\n", &line_tokenizer);
    while(line) {
      info("(line ): %s", line);
      if (line[0] == command_starter) {
        char *token = strtok_r(line, " ", &space_tokenizer);
        while (token) {
          if (strcmp("font", token) == 0) {
            if (!style) style = calloc(1, sizeof(style_item));
            token = strtok_r(0, " ", &space_tokenizer);
            style->size = strtof(token, 0); // require `. font [size]`
            while (token) {
              for (int i = 0; i < num_families; ++i)
                if (strcmp(family[i].name, token) == 0) {
                  style->family = family[i].f;
                  break;
                }
              token = strtok_r(0, " ", &space_tokenizer);
            }
            continue; // we at all tokens on the font line
          }
          token = strtok_r(0, " ", &space_tokenizer);
        }
        style = 0;
      } else if (line[0] == slide_starter) {
        strtok_r(line, " ", &space_tokenizer); // eat space tokens
        char *title = strtok_r(0, "\n", &space_tokenizer);
        slide = calloc(1, sizeof(slide_item));
        slide->title = title;
        slide->bg_color = cf4(0,0,0,.8);
        push(the_show->slides, slide);
      } else {
        if (!slide) {
          error("no slide before -- %s", line);
          continue;
        }

        item = calloc(1, sizeof(text_item));
        push(slide->items, item);
        item->fg_color =   cf4(1, 1, 1, 1);
        item->type =       text_slide;
        item->y =          .5;
        size_t len =       strlen(line);
        item->text =       malloc(len * sizeof(char) + 1);
        memcpy(item->text, line, len + 1);
        push(slide->styles, style ?: initial);
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
  }

  return the_show;
}

void render_slide(SDL_Renderer *renderer, int w, int h, slide_show *show_baby) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  slide_item *current_slide = show_baby->slides[show_baby->index];
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
    int margins = (int)(w * .05f);
    switch(style->align) {
      default:
      case left:
        slide_text_rect.x += margins;
        break;
      case center:
        slide_text_rect.x += w/2 - slide_text_rect.w/2;
        break;
      case right:
        slide_text_rect.x = w - slide_text_rect.w - margins;
        break;
    }
    TTF_CloseFont(f);
    SDL_RenderCopy(renderer, slide_text, 0, &slide_text_rect);
    SDL_DestroyTexture(slide_text);
  }
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