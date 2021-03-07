#ifndef UI_H
#define UI_H

#include "main.h"
#include "entity.h"

typedef enum {
  UI_STATE_NONE,
  UI_STATE_INVENTORY,
  UI_STATE_CHARACTER,
} ui_state_e;

extern int ui_rendering;
extern int ui_state;

void ui_init();

void ui_render();

void ui_print_entity(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print_entity_up(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print_entity_down(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print(const char *str, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_reset();

void ui_inventory(entity_t *e);

void ui_fire(entity_t *e);

void ui_use(entity_t *e);

void ui_character();

#endif // UI_H