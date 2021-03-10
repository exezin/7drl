#ifndef UI_H
#define UI_H

#include "main.h"
#include "entity.h"

typedef enum {
  UI_STATE_NONE,
  UI_STATE_INVENTORY,
  UI_STATE_FIRE,
  UI_STATE_USE,
  UI_STATE_ITEM,
  UI_STATE_AIM,
  UI_STATE_DEAD,
} ui_state_e;

extern int ui_rendering;
extern int ui_state;
extern int ui_count;
extern char ui_previous[128];

extern tilesheet_packet_t ui_tiles;

void ui_init();

void ui_render();

void ui_print_entity(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print_entity_up(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_print_entity_down(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_popup(entity_t *e, const char *str, u8 r, u8 g, u8 b, u8 a);

int ui_print(const char *str, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a);

void ui_reset();

void ui_inventory(entity_t *e);

void ui_fire(entity_t *e);

void ui_use(entity_t *e);

void ui_item(entity_t *e, int item);

void ui_character();

void ui_dead();

#endif // UI_H