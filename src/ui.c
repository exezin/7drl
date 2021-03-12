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
int ui_maxlen = 0;
tilesheet_packet_t ui_tiles;

/*
  HERE BE DRAGONS
  PLEASE LEAVE
*/

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

int ui_x = 0, ui_y = 0, ui_count = 0;
char ui_previous[128];
void ui_popup(entity_t *e, const char *str, u8 r, u8 g, u8 b, u8 a)
{
  ui_y = 2;
  char buf[128];
  sprintf(buf, "%s", str);

  if (!strcmp(buf, ui_previous)) {
    ui_count++;
    sprintf(buf, "%s x%i", str, ui_count);
  } else {
    ui_count = 0;
  }
  
  sprintf(ui_previous, "%s", str);

  int len = strlen(buf);

  if (e->position.to[0] > (TILES_X/2))
    ui_x = 2;
  else
    ui_x = TILES_X-(len+2);
  
  ui_print(buf, ui_x, ui_y, r, g, b, a);
}

int ui_print(const char *str, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a)
{
  // ui_rendering = 1;

  int ix = x;
  int count = 0, lines = 0;
  for (int i=0; i<strlen(str); i++) {
    size_t c = str[i];

    if (c == '\n')
      break;

    if (c > 63 && c < 91) {
      c -= 64;
    } else if (c >= '0' && c <= '9') {
      c -= '0'-27;
    } else {
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
        case '#': {
          c = 39;
          break;
        }
        case '/': {
          c = 48;
          break;
        }
        case ',': {
          c = 64;
          break;
        }
        case '<': {
          c = 75;
          break;
        }
        case '>': {
          c = 74;
          break;
        }
        case '!': {
          c = 37;
          break;
        }
        case '.': {
          c = 39;
          break;
        }
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
    count++;

    if (ui_maxlen && count > ui_maxlen) {
      count = 0;
      y++;
      x = ix;
      lines++;
    }

    if (x > ui_tiles.w)
      break;
  }

  return lines;
}

void ui_inventory(entity_t *e)
{
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_INVENTORY_____________}", x, y++, 100, 100, 120, 255);
  char buf[128];
  int index = 0;
  for (int i=0; i<INVENTORY_MAX; i++) {
    for (int j=0; j<width; j++)
      ui_print(" ", x+j, y, 100, 100, 120, 255);

    // borders
    ui_print("|", x, y, 100, 100, 120, 255);
    ui_print("|", x+width, y, 100, 100, 120, 255);

    int item = e->inventory.items[i];
    int tile = 0;
    if (item > ITEM_POTION_START && item < ITEM_POTION_END)
      tile = BLOCK_POTION;
    if (item > ITEM_SCROLL_START && item < ITEM_SCROLL_END)
      tile = BLOCK_SCROLL;
    if (item > ITEM_GEAR_START && item < ITEM_GEAR_END)
      tile = BLOCK_GEAR;
    if (item > ITEM_WAND_START && item < ITEM_WAND_END)
      tile = BLOCK_WAND;
    if (item == ITEM_KEY)
      tile = 68+9;
    if (item == ITEM_NONE)
      continue;

    int on = e->inventory.equipt[i];

    // item name
    sprintf(buf, "%c)%s", 'A'+i, item_info[item].name);
    ui_print(buf, x+1, y, 100, 100, 120, 255);
    int len = strlen(buf) + 2;
    if (item > ITEM_GEAR_START) {
      sprintf(buf, "( ");
      ui_print(buf, x+width-2, y, 120, 120, 120, 255);
      if (on) {
        sprintf(buf, "%c", 42);
        ui_print(buf, x+width-1, y, 120, 255, 120, 255);
      }
    }

    ui_tiles.tiles[(y*ui_tiles.w)+x+len].tile = tile;
    if (!item_info[item].identified) {
      ui_tiles.tiles[(y*ui_tiles.w)+x+len].tile = 43;
      ui_tiles.tiles[(y*ui_tiles.w)+x+len].r = 160;
    }
    // ui_tiles.tiles[(y*ui_tiles.w)+x+len].r = 255;
    // ui_tiles.tiles[(y*ui_tiles.w)+x+len].b = 255;

    y++;
  }
  ui_print("[_______________________]", x, y, 100, 100, 120, 255);

  ui_rendering = 1;
  ui_state = UI_STATE_INVENTORY;
}

void ui_fire(entity_t *e)
{
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_FIRE__________________}", x, y++, 100, 100, 120, 255);
  char buf[128];
  int index = 0;
  for (int i=0; i<INVENTORY_MAX; i++) {
    for (int j=0; j<width; j++)
      ui_print(" ", x+j, y, 100, 100, 120, 255);

    // borders
    ui_print("|", x, y, 100, 100, 120, 255);
    ui_print("|", x+width, y, 100, 100, 120, 255);

    int item = e->inventory.items[i];
    if (item == ITEM_NONE)
      continue;

    if (item > ITEM_POTION_START && item < ITEM_POTION_END)
      continue;
    if (item > ITEM_SCROLL_START && item < ITEM_SCROLL_END)
      continue;
    if (item > ITEM_GEAR_START && item < ITEM_GEAR_END)
      continue;

    int on = e->inventory.equipt[i] ? 42 : BLOCK_NONE;

    // item name
    sprintf(buf, "%c)%s", 'A'+i, item_info[item].name);
    ui_print(buf, x+1, y, 100, 100, 120, 255);
    // sprintf(buf, "(%c", on);
    // ui_print(buf, x+width-2, y, 255, 255, 255, 255);
    y++;
  }
  ui_print("[_______________________]", x, y, 100, 100, 120, 255);

  ui_rendering = 1;
  ui_state = UI_STATE_FIRE;
}

void ui_use(entity_t *e)
{
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_USE___________________}", x, y++, 100, 100, 120, 255);
  char buf[128];
  int index = 0;
  for (int i=0; i<INVENTORY_MAX; i++) {
    for (int j=0; j<width; j++)
      ui_print(" ", x+j, y, 100, 100, 120, 255);

    // borders
    ui_print("|", x, y, 100, 100, 120, 255);
    ui_print("|", x+width, y, 100, 100, 120, 255);

    int item = e->inventory.items[i];
    if (item == ITEM_NONE)
      continue;

    int tile = 0;
    if (item > ITEM_WAND_START && item < ITEM_WAND_END)
      continue;
    if (item > ITEM_GEAR_START && item < ITEM_GEAR_END)
      continue;

    int on = e->inventory.equipt[i] ? 42 : BLOCK_NONE;

    // item name
    sprintf(buf, "%c)%s", 'A'+i, item_info[item].name);
    ui_print(buf, x+1, y, 100, 100, 120, 255);
    // sprintf(buf, "(%c", on);
    // ui_print(buf, x+width-2, y, 255, 255, 255, 255);
    y++;
  }
  ui_print("[_______________________]", x, y, 100, 100, 120, 255);

  ui_rendering = 1;
  ui_state = UI_STATE_USE;
}

void ui_item(entity_t *e, int item)
{
  ui_reset();

  if (!e->inventory.items[item])
    return;

  int tile = 0; int itemid = e->inventory.items[item];
  if (itemid > ITEM_POTION_START && itemid < ITEM_POTION_END)
    tile = BLOCK_POTION;
  if (itemid > ITEM_SCROLL_START && itemid < ITEM_SCROLL_END)
    tile = BLOCK_SCROLL;
  if (itemid > ITEM_GEAR_START && itemid < ITEM_GEAR_END)
    tile = BLOCK_GEAR;
  if (itemid > ITEM_WAND_START && itemid < ITEM_WAND_END)
    tile = BLOCK_WAND;
  if (itemid == ITEM_KEY)
    tile = 68+9;

  char buf[128];
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_______________________}", x, y, 100, 100, 120, 255);
  
  sprintf(buf, ">%s<", item_info[e->inventory.items[item]].name);
  ui_print(buf, x+2, y++, 100, 100, 120, 255);

  ui_print("|                       |", x, y, 100, 100, 120, 255);

  // description
  ui_maxlen = 22;
  sprintf(buf, "%s", item_info[e->inventory.items[item]].description);
  int count = ui_print(buf, x+1, y, 100, 100, 120, 255);
  ui_maxlen = 0;
  for (int i=0; i<=count+1; i++) {
    ui_print("|                       |", x, y++, 100, 100, 120, 255);
  }
  ui_maxlen = 22;
  ui_print(buf, x+1, y-count-2, 100, 100, 120, 255);
  ui_maxlen = 0;

  // uses
  if (itemid < ITEM_GEAR_START && item_info[itemid].identified) {
    ui_print("|                       |", x, y, 100, 100, 120, 255);
    sprintf(buf, "IT HAS %i USES LEFT", e->inventory.uses[item]);
    ui_print(buf, x+1, y++, 100, 100, 120, 255);
    sprintf(buf, "%i", e->inventory.uses[item]);
    ui_print(buf, x+8, y-1, 120, 255, 120, 255);
  } else if (itemid > ITEM_GEAR_WEAPON_START) {
    ui_print("|                       |", x, y, 100, 100, 120, 255);
    sprintf(buf, "IT HAS A BASE DAMAGE");
    ui_print(buf, x+1, y++, 100, 100, 120, 255);
    ui_print("|                       |", x, y, 100, 100, 120, 255);
    sprintf(buf, "OF ABOUT %i PER TURN", item_info[e->inventory.items[item]].damage);
    ui_print(buf, x+1, y++, 100, 100, 120, 255);
    sprintf(buf, "%i", item_info[e->inventory.items[item]].damage);
    ui_print(buf, x+10, y-1, 120, 255, 120, 255);
  } else if (itemid > ITEM_GEAR_START && itemid < ITEM_GEAR_WEAPON_START) {
    ui_print("|                       |", x, y, 100, 100, 120, 255);
    sprintf(buf, "IT WOULD PROVIDE YOU");
    ui_print(buf, x+1, y++, 100, 100, 120, 255);
    ui_print("|                       |", x, y, 100, 100, 120, 255);
    sprintf(buf, "WITH %i ARMOR", item_info[e->inventory.items[item]].armor);
    ui_print(buf, x+1, y++, 100, 100, 120, 255);
    sprintf(buf, "%i", item_info[e->inventory.items[item]].armor);
    ui_print(buf, x+6, y-1, 120, 255, 120, 255);
  }

  if (!item_info[itemid].identified)
    y--;

  ui_print("[_>A)USE<______>B)DROP<_]", x, y, 100, 100, 120, 255);
  ui_print("A)USE", x+3, y, 120, 120, 255, 255);
  ui_print("B)DROP", x+16, y, 120, 120, 255, 255);

  ui_tiles.tiles[(y*ui_tiles.w)+x+12].tile = tile;
  ui_tiles.tiles[(y*ui_tiles.w)+x+12].r = 255;
  ui_tiles.tiles[(y*ui_tiles.w)+x+12].b = 255;
  ui_tiles.tiles[(y*ui_tiles.w)+x+13].tile = 75;
  ui_tiles.tiles[(y*ui_tiles.w)+x+11].tile = 74;

  ui_rendering = 1;
  ui_state = UI_STATE_ITEM;
}

void ui_character(entity_t *e)
{
  char buf[512];
  // render borders
  int r = 100, g = 100, b = 120, a = 255;
  for (int i=0; i<TILES_Y; i++) {
    a = 255;//(100 * (cos((float)i * 0.2) * 0.5 + 0.5));
    ui_print("|", 0, i, r, g, b, a);
    ui_print("|", TILES_X, i, r, g, b, a);
  }
  for (int i=0; i<TILES_X; i++) {
    a = 255;
    ui_print("_", i, 0, r, g, b, a);
    ui_print("_", i, TILES_Y, r, g, b, a);
  }
  ui_print("{", 0, 0, r, g, b, a);
  ui_print("}", TILES_X, 0, r, g, b, a);
  ui_print("[", 0, TILES_Y, r, g, b, a);
  ui_print("]", TILES_X, TILES_Y, r, g, b, a);

  sprintf(buf, ">LEVEL %i<", e->stats.level);
  ui_print(buf, (TILES_X/2)-strlen(buf)/2, 0, 100, 100, 120, 255);

  // health
  sprintf(buf, ">,%i/%i,<", MAX(e->stats.health, 0), e->stats.max_health);
  ui_print(buf, (TILES_X/2)-strlen(buf)/2, TILES_Y, 100, 100, 120, 255);

  // health
  sprintf(buf, ">DEPTH %i<", 4 - dungeon_depth);
  ui_print(buf, (TILES_X)-strlen(buf)-1, TILES_Y, 100, 100, 120, 255);

  if (tile_on) { 
    buf[0] = '\0';
    switch (tile_on) {
      case BLOCK_DOOR:
      case BLOCK_DOOR_OPEN:
      case BLOCK_FLOOR: {
        sprintf(buf, ">COBBLESTONE<");
        break;
      }
      case BLOCK_FLOOR+1: {
        sprintf(buf, ">GRASS<");
        break;
      }
      case BLOCK_WATER: {
        sprintf(buf, ">WATER<");
        break;
      }
      case BLOCK_STAIRS: {
        sprintf(buf, ">STAIRS UP<");
        break;
      }
    }
    ui_print(buf, 1, TILES_Y, 100, 100, 120, 255);
  }
}

void ui_inspect(entity_t *e)
{
  ui_reset();

  if (strlen(e->description) < 1)
    return;

  int tile = e->renderable.tile;

  char buf[128];
  int width = 24;
  int x = (TILES_X/2) - (width/2), y = (TILES_Y/2) - (INVENTORY_MAX / 2);

  ui_print("{_______________________}", x, y, 100, 100, 120, 255);
  
  sprintf(buf, ">%s<", e->name);
  ui_print(buf, x+2, y++, 100, 100, 120, 255);

  ui_print("|                       |", x, y, 100, 100, 120, 255);

  // description
  ui_maxlen = 22;
  sprintf(buf, "%s", e->description);
  int count = ui_print(buf, x+1, y, 100, 100, 120, 255);
  ui_maxlen = 0;
  for (int i=0; i<=count+1; i++) {
    ui_print("|                       |", x, y++, 100, 100, 120, 255);
  }
  ui_maxlen = 22;
  ui_print(buf, x+1, y-count-2, 100, 100, 120, 255);
  ui_maxlen = 0;
  // y--;

  ui_print("|                       |", x, y, 100, 100, 120, 255);
  if (e->ai.hostile)
    sprintf(buf, "HOSTILE YES");
  else
    sprintf(buf, "HOSTILE NO");
  ui_print(buf, x+1, y++, 100, 100, 120, 255);

  ui_print("|                       |", x, y, 100, 100, 120, 255);
  sprintf(buf, "HEALTH  %i/%i", e->stats.health, e->stats.max_health);
  ui_print(buf, x+1, y++, 100, 100, 120, 255);

  ui_print("|                       |", x, y, 100, 100, 120, 255);
  sprintf(buf, "SPEED   %i", (int)(100 * e->speed.speed));
  ui_print(buf, x+1, y++, 100, 100, 120, 255);
  
  ui_print("[_______________________]", x, y, 100, 100, 120, 255);

  ui_tiles.tiles[(y*ui_tiles.w)+x+12].tile = tile;
  ui_tiles.tiles[(y*ui_tiles.w)+x+12].r = 255;
  ui_tiles.tiles[(y*ui_tiles.w)+x+12].b = 255;
  ui_tiles.tiles[(y*ui_tiles.w)+x+13].tile = 75;
  ui_tiles.tiles[(y*ui_tiles.w)+x+11].tile = 74;

  ui_rendering = 1;
  ui_state = UI_STATE_INSPECT;
}

void ui_dead()
{
  ui_state = UI_STATE_DEAD; 
  char buf[128];
  sprintf(buf, "YOU HAVE DIED ON LEVEL %i", 4 - dungeon_depth);
  ui_print(buf, (TILES_X/2)-strlen(buf)/2, (TILES_Y/2)-2, 255, 120, 120, 255);
  sprintf(buf, "PRESS Q TO RETURN TO THE MENU");
  ui_print(buf, (TILES_X/2)-strlen(buf)/2, (TILES_Y/2)-1, 255, 120, 120, 255);
}

void ui_end()
{
  ui_state = UI_STATE_END; 
  char buf[128];
  sprintf(buf, "YOU HAVE ESCAPED THE DUNGEON");
  ui_print(buf, (TILES_X/2)-strlen(buf)/2, (TILES_Y/2)-2, 255, 255, 120, 255);
  sprintf(buf, "PRESS Q TO RETURN TO THE MENU");
  ui_print(buf, (TILES_X/2)-strlen(buf)/2, (TILES_Y/2)-1, 255, 255, 120, 255);
}

void ui_reset()
{
  memset(ui_tiles.tiles, 0, sizeof(tile_t) * ui_tiles.w * ui_tiles.h);
  ui_rendering = 0;
  ui_state = UI_STATE_NONE;
}