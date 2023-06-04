#include <SDL2/SDL.h>

void sdl_error() {
  const char *error = SDL_GetError();
  if (*error) {
    SDL_Log("SDL error: %s", error);
    SDL_ClearError();
  }
}

/* this window and comments are added by justin Sat Oct 16 13:16:19 MDT 2021 */
int main() {
  SDL_Surface *screen;
  SDL_Window *window;
  SDL_Surface *image;
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  SDL_Log("initializing");

  if (!(SDL_SetHint(SDL_HINT_VIDEO_X11_XRANDR, "1"))) {
    fprintf(stderr, "hint died");
    exit(EXIT_FAILURE);
  }

  // sdl's file i/o and threading subsystems are initialized by default
  // event handling subsystems are initialized here
  // this will fail on some systems if SDL_Main is not defined as an etry point
  // SDL_Init simply forwars to SDL_InitSubSystem and can be used interchangably
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    fprintf(stderr, "SDL_Init failure");
    sdl_error();
    exit(EXIT_FAILURE);
  }

  // should be called before an sdl application exits
  // to safely shutdown all subsystems (including default ones)
  // should be called even if you have already shutdown each initialized sybsystem
  // You can use this function with atexit() to ensure
  // that it is run when your application is shutdown
  // (but not wise for library/dynamic code)
  atexit(SDL_Quit);

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "initialized");

  window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

  screen = SDL_GetWindowSurface(window);

  image = SDL_LoadBMP("../res/car.bmp");
  SDL_BlitSurface(image, NULL, screen, NULL);
  SDL_FreeSurface(image);

  // this works just like SDL_Flip() in SDL 1.2
  SDL_UpdateWindowSurface(window);

  SDL_Delay(2000);
  SDL_DestroyWindow(window);
  return 0;
}