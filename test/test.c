#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF

#include "../src/lib/stb_image.h"

int main() {
  int x;
  int y;
  int n_chans;
  int desired = 0; // unset?
  char *images[] = {
      "blue-chiron.jpg",
      "cursor.png",
      "blue-lambo.jpg",
      "car.bmp",
      "grey-subi.jpg",
      "neon-buggati.jpg",
      "green-bently.jpg",
      "red-chiron.jpg",
      "pink-lambo.jpg",
  };
  char str[1000];
  for (int i = 0; i < sizeof(images) / sizeof(images[0]); ++i) {
    sprintf(str, "../res/%s", images[i]);
    stbi_uc *image = stbi_load(str, &x, &y, &n_chans, desired);
    printf("image %s has x%d y%d at %p with %d chans\n", str, x, y, (void*)image, n_chans);
    stbi_image_free(image);
  }
}
