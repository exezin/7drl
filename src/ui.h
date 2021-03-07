#ifndef UI_H
#define UI_H

#include "main.h"
#include "entity.h"

extern int ui_rendering;

void ui_init();

void ui_render();

void ui_print_entity(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print_entity_up(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print_entity_down(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print(const char *str, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_reset();

void ui_keypress(int key);

void ui_mousepress(int x, int y);

#endif // UI_H