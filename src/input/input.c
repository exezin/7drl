#include "input/input.h"
#include "game.h"

int mouse_x = 0, mouse_y = 0;
u8 keys_down[SDL_NUM_SCANCODES];
u8 buttons_down[16];

void input_event(SDL_Event *event)
{
  switch (event->type) {
    // keyboard
    case SDL_KEYDOWN: {
      keys_down[event->key.keysym.scancode] = 1;
      // if (event->key.repeat == 0)
        game_keypressed(event->key.keysym.scancode);
      break;
    }
    case SDL_KEYUP: {
      keys_down[event->key.keysym.scancode] = 0;
      break;
    }

    // mouse
    case SDL_MOUSEBUTTONDOWN: {
      buttons_down[event->button.button] = 1;
      if (event->button.state == SDL_PRESSED)
        game_mousepressed(event->button.button);
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      buttons_down[event->button.button] = 0;
      break;
    }
    case SDL_MOUSEWHEEL: {
      game_mousewheel(event->wheel.x, event->wheel.y);
      break;
    }
    case SDL_MOUSEMOTION: {
      game_mousemotion(event->motion.xrel, event->motion.yrel);
      break;
    }
  }
}

void input_update()
{
  if (SDL_GetRelativeMouseMode())
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
  else
    SDL_GetMouseState(&mouse_x, &mouse_y);
}