/* input
  Handles all user input, such as
  keyboard, mouse, gamepad, etc.

  Key codes will work for every keyboard
  layout.  They work by mapping each key
  of a US keyboard layout to the closest
  key on whatever layout the user has.

  For example, Q is the top-left character
  key for all keyboard layouts.

  Don't manually use the callbacks here,
  instead set the engine function pointers
  and your functions will be called when
  an event fires.  See engine.h for details.
*/

#ifndef INPUT_H
#define INPUT_H

#include "main.h"
#include "input/scancodes.h"

#include <inttypes.h>
#include <SDL2/SDL.h>

extern int mouse_x, mouse_y;
extern u8 keys_down[SDL_NUM_SCANCODES];
extern u8 buttons_down[16];

typedef struct {
  u32 scancode;
  void (*action)(void);
} keybind_t;

static inline int key_to_num(SDL_Scancode key) {
  int use_item = 0;
  switch (key) {
    case SDL_SCANCODE_A: {
      use_item = 0;
      break;
    }
    case SDL_SCANCODE_B: {
      use_item = 1;
      break;
    }
    case SDL_SCANCODE_C: {
      use_item = 2;
      break;
    }
    case SDL_SCANCODE_D: {
      use_item = 3;
      break;
    }
    case SDL_SCANCODE_E: {
      use_item = 4;
      break;
    }
    case SDL_SCANCODE_F: {
      use_item = 5;
      break;
    }
    case SDL_SCANCODE_G: {
      use_item = 6;
      break;
    }
    case SDL_SCANCODE_H: {
      use_item = 7;
      break;
    }
    case SDL_SCANCODE_I: {
      use_item = 8;
      break;
    }
    case SDL_SCANCODE_J: {
      use_item = 9;
      break;
    }
    case SDL_SCANCODE_K: {
      use_item = 10;
      break;
    }
    case SDL_SCANCODE_L: {
      use_item = 11;
      break;
    }
    case SDL_SCANCODE_M: {
      use_item = 12;
      break;
    }
    case SDL_SCANCODE_N: {
      use_item = 13;
      break;
    }
    case SDL_SCANCODE_O: {
      use_item = 14;
      break;
    }
    case SDL_SCANCODE_P: {
      use_item = 15;
      break;
    }
    default: {
      break;
    }
  }

  return use_item;
}

/**
 * [input_event handles input events]
 * @param event [SDL_Event pointer]
 */
void input_event(SDL_Event *event);

/**
 * [input_update handles more frequent input updates]
 */
void input_update();

#endif // INPUT_H