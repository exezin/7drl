#include <time.h>
#include "game.h"
#include "gen.h"
#include "entity.h"
#include "ui.h"
#include "render/render.h"
#include "render/vga.h"
#include "input/input.h"

int ui_rendering = 0;
int ui_state = 0;
tilesheet_packet_t ui_tiles;

void ui_init()
{
  // tilemap specifically for entities
  ui_tiles.x = 0, ui_tiles.y = 0;
  ui_tiles.zoom = 1;
  ui_tiles.w = (WINDOW_WIDTH / TILE_RWIDTH), ui_tiles.h = (WINDOW_HEIGHT / TILE_RHEIGHT);
  ui_tiles.rx = 0, ui_tiles.rw = ui_tiles.w;
  ui_tiles.ry = 0, ui_tiles.rh = ui_tiles.h;
  ui_tiles.tiles = calloc(1, sizeof(tile_t) * ui_tiles.w * ui_tiles.h);
}

void ui_render()
{
  render_tilemap(&ui_tiles);
}

void ui_print_entity(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a)
{
  if (e->position.to[1] > ui_tiles.h/2)
    ui_print_entity_up(e, str, y, r, g, b, a);
  else 
    ui_print_entity_down(e, str, y, r, g, b, a);
}

void ui_print_entity_up(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a)
{
  int len = (int)strlen(str);
  int dist = y;
  int x = CLAMP(e->position.to[0] - (len / 2), 0, (int)(ui_tiles.w - len - 1));
  y = e->position.to[1] - dist;

  for (int i=0; i<dist; i++) {
    ui_print("x", e->position.to[0], y+i, r, g, b, a);
  }
  ui_print(str, x, y, r, g, b, a);
}

void ui_print_entity_down(entity_t *e, const char *str, u32 y, u8 r, u8 g, u8 b, u8 a)
{
  int len = (int)strlen(str);
  int dist = y;
  int x = CLAMP(e->position.to[0] - (len / 2), 0, (int)(ui_tiles.w - len - 1));
  P_DBG("%i\n", x);
  y = e->position.to[1] + dist;

  for (int i=0; i<dist; i++) {
    ui_print("x", e->position.to[0], y-i, r, g, b, a);
  }
  ui_print(str, x, y, r, g, b, a);
}

void ui_print(const char *str, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a)
{
  // ui_rendering = 1;

  for (int i=0; i<strlen(str); i++) {
    size_t c = str[i];

    if (c == '\n')
      break;

    if (c > 63 && c < 91)
      c -= 64;

    switch (c) {
      case ' ': {
        c = 59;
        break;
      }
      case '?': {
        c = 43;
        break;
      }
      case '|': {
        c = 65;
        break;
      }
      case '_': {
        c = 66;
        break;
      }
      case 'x': {
        c = 67;
        break;
      }
      case '{': {
        c = 68;
        break;
      }
      case '}': {
        c = 69;
        break;
      }
      case '[': {
        c = 70;
        break;
      }
      case ']': {
        c = 71;
        break;
      }
      case ')': {
        c = 40;
        break;
      }
      case '(': {
        c = 41;
        break;
      }
    }

    x = CLAMP(x, 0, ui_tiles.w-1);
    y = CLAMP(y, 0, ui_tiles.h-1);

    u32 index = (y * ui_tiles.w) + x;
    ui_tiles.tiles[index].tile = c;
    ui_tiles.tiles[index].r    = r;
    ui_tiles.tiles[index].g    = g;
    ui_tiles.tiles[index].b    = b;
    ui_tiles.tiles[index].a    = a;
    x++;

    if (x > ui_tiles.w)
      break;
  }
}

void ui_inventory(entity_t *e)
{
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_EQUIP_________________}", x, y++, 255, 255, 255, 255);
  char buf[128];
  int index = 0;
  for (int i=0; i<INVENTORY_MAX; i++) {
    for (int j=0; j<width; j++)
      ui_print(" ", x+j, y, 255, 255, 255, 255);

    // borders
    ui_print("|", x, y, 255, 255, 255, 255);
    ui_print("|", x+width, y, 255, 255, 255, 255);

    int item = e->inventory.items[i];
    if (item == ITEM_NONE)
      continue;

    int tile = 0;
    if (item > ITEM_POTION_START && item < ITEM_POTION_END)
      tile = BLOCK_POTION;
    if (item > ITEM_SCROLL_START && item < ITEM_SCROLL_END)
      tile = BLOCK_SCROLL;
    if (item > ITEM_WAND_START && item < ITEM_WAND_END)
      tile = BLOCK_WAND;
    if (item > ITEM_GEAR_START && item < ITEM_GEAR_END)
      tile = BLOCK_GEAR;
    else
      continue;

    int on = e->inventory.equipt[i] ? 42 : BLOCK_NONE;

    // item name
    sprintf(buf, "%c)%c%s", 'A'+i, tile, item_info[item].name);
    ui_print(buf, x+1, y, 255, 255, 255, 255);
    sprintf(buf, "(%c", on);
    ui_print(buf, x+width-2, y, 255, 255, 255, 255);
    y++;
  }
  ui_print("[_______________________]", x, y, 255, 255, 255, 255);

  ui_rendering = 1;
  ui_state = UI_STATE_INVENTORY;
}

void ui_fire(entity_t *e)
{
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_FIRE__________________}", x, y++, 255, 255, 255, 255);
  char buf[128];
  int index = 0;
  for (int i=0; i<INVENTORY_MAX; i++) {
    for (int j=0; j<width; j++)
      ui_print(" ", x+j, y, 255, 255, 255, 255);

    // borders
    ui_print("|", x, y, 255, 255, 255, 255);
    ui_print("|", x+width, y, 255, 255, 255, 255);

    int item = e->inventory.items[i];
    if (item == ITEM_NONE)
      continue;

    int tile = 0;
    if (item > ITEM_POTION_START && item < ITEM_POTION_END)
      continue;
    if (item > ITEM_SCROLL_START && item < ITEM_SCROLL_END)
      continue;
    if (item > ITEM_WAND_START && item < ITEM_WAND_END)
      tile = BLOCK_WAND;
    if (item > ITEM_GEAR_START && item < ITEM_GEAR_END)
      continue;

    int on = e->inventory.equipt[i] ? 42 : BLOCK_NONE;

    // item name
    sprintf(buf, "%c)%c%s", 'A'+i, tile, item_info[item].name);
    ui_print(buf, x+1, y, 255, 255, 255, 255);
    // sprintf(buf, "(%c", on);
    // ui_print(buf, x+width-2, y, 255, 255, 255, 255);
    y++;
  }
  ui_print("[_______________________]", x, y, 255, 255, 255, 255);

  ui_rendering = 1;
  ui_state = UI_STATE_INVENTORY;
}

void ui_use(entity_t *e)
{
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_USE___________________}", x, y++, 255, 255, 255, 255);
  char buf[128];
  int index = 0;
  for (int i=0; i<INVENTORY_MAX; i++) {
    for (int j=0; j<width; j++)
      ui_print(" ", x+j, y, 255, 255, 255, 255);

    // borders
    ui_print("|", x, y, 255, 255, 255, 255);
    ui_print("|", x+width, y, 255, 255, 255, 255);

    int item = e->inventory.items[i];
    if (item == ITEM_NONE)
      continue;

    int tile = 0;
    if (item > ITEM_POTION_START && item < ITEM_POTION_END)
      tile = BLOCK_POTION;
    if (item > ITEM_SCROLL_START && item < ITEM_SCROLL_END)
      tile = BLOCK_SCROLL;
    if (item > ITEM_WAND_START && item < ITEM_WAND_END)
      continue;
    if (item > ITEM_GEAR_START && item < ITEM_GEAR_END)
      continue;

    int on = e->inventory.equipt[i] ? 42 : BLOCK_NONE;

    // item name
    sprintf(buf, "%c)%c%s", 'A'+i, tile, item_info[item].name);
    ui_print(buf, x+1, y, 255, 255, 255, 255);
    // sprintf(buf, "(%c", on);
    // ui_print(buf, x+width-2, y, 255, 255, 255, 255);
    y++;
  }
  ui_print("[_______________________]", x, y, 255, 255, 255, 255);

  ui_rendering = 1;
  ui_state = UI_STATE_INVENTORY;
}

void ui_character(entity_t *e)
{
  int x = 1;
  if (e->position.to[0] < TILES_X/2)
    x = TILES_X-10;

  int y = 2;
  ui_print("LEVEL", x, y, 255, 255, 255, 255);

  ui_rendering = 1;
  ui_state = UI_STATE_CHARACTER;
}

void ui_reset()
{
  memset(ui_tiles.tiles, 0, sizeof(tile_t) * ui_tiles.w * ui_tiles.h);
  ui_rendering = 0;
  ui_state = UI_STATE_NONE;
}