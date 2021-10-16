#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF

#include <SDL2/SDL.h>
#include "../src/lib/stb_image.h"

int main() {
  int x;
  int y;
  int n_chans;
  int desired = 0; // unset?
  char *images[] = {
      "blue-chiron.jpg",
      "cursor.png",
      // "blue-lambo.jpg",
      // "car.bmp",
      // "grey-subi.jpg",
      // "neon-buggati.jpg",
      // "green-bently.jpg",
      // "red-chiron.jpg",
      // "pink-lambo.jpg",
  };
  char str[1000];
  for (int i = 0; i < sizeof(images) / sizeof(images[0]); ++i) {
    sprintf(str, "../res/%s", images[i]);
    stbi_uc *image = stbi_load(str, &x, &y, &n_chans, desired);
    stbi_image_free(image);
    int bits_ppix = n_chans * 8;
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
            image, x, y, bits_ppix,
            x * 4, SDL_PIXELFORMAT_ABGR8888);
    printf("image %s has x%d y%d at %p with %d chans @ %p\n", str, x, y, (void *) image, n_chans, surf);
  }
}
