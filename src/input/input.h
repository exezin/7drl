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