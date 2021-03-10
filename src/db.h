#ifndef DB_H
#define DB_H

// default values
#define WINDOW_TITLE  "rl"
#define WINDOW_WIDTH  477 // 432
#define WINDOW_HEIGHT 306 // 261
#define WINDOW_FLAGS  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE

// tilemap stuff
#define TILE_WIDTH   9
#define TILE_HEIGHT  9
#define TILE_U       11
#define TILE_V       11
#define TILE_RWIDTH  TILE_WIDTH
#define TILE_RHEIGHT TILE_HEIGHT
#define TILES_X      (WINDOW_WIDTH/TILE_RWIDTH)
#define TILES_Y      (WINDOW_HEIGHT/TILE_RHEIGHT)
#define TILES_NUM   (TILES_X*TILES_Y)

// minimum energy required for a turn
#define ENERGY_MIN 1.0f

#define PROJECTILE_SPEED 0.035f;

#define INVENTORY_MAX 15

typedef enum {
  IDENT_UNKNOWN,
  IDENT_PLAYER,
  IDENT_NPC,
  IDENT_CONTAINER,

  IDENT_NUM
} ident_e;

typedef enum {
  ITEM_NONE,

  // one time use
  ITEM_POTION_START,
  ITEM_POTION_HEALING,
  ITEM_POTION_END,

  // one time use
  ITEM_SCROLL_START,
  ITEM_SCROLL_MAPPING,
  ITEM_SCROLL_END,

  // multi time use
  ITEM_WAND_START,
  ITEM_WAND_FIREBOLT,
  ITEM_WAND_END,

  // wearables
  ITEM_GEAR_START,
  ITEM_GEAR_CHAINHELM,

  ITEM_GEAR_WEAPON_START,
  ITEM_GEAR_IRONDAGGER,
  ITEM_GEAR_END,

  ITEM_NUM
} item_e;

typedef enum {
  SLOT_ARMS,
  SLOT_HANDS,
  SLOT_HEAD,
  SLOT_CHEST,
  SLOT_LEGS,
  SLOT_FEET,
  SLOT_RIGHT,
  SLOT_LEFT,
  
  SLOT_NUM
} slot_e;

typedef enum {
  ELEMENT_FIRE,

  ELEMENT_NUM
} element_e;

typedef struct {
  char description[512];
  char name[20];
  int base_uses, identified;
  int armor, damage, slot;
  int range, element;
} item_info_t;
extern item_info_t item_info[ITEM_NUM];

typedef enum {
  BLOCK_NONE = 0,
  BLOCK_WALL = 44,
  BLOCK_WALL_V = 45,
  BLOCK_FLOOR = 46,
  BLOCK_DOOR = 60,
  BLOCK_DOOR_OPEN = 61,
  BLOCK_HOLE,
  BLOCK_LAVA,
  BLOCK_WATER = 51,
  BLOCK_WATER_DEEP = 52,
  BLOCK_PILLAR,
  BLOCK_SWORD = 53,
  BLOCK_POTION,
  BLOCK_SCROLL,
  BLOCK_WAND,
  BLOCK_GEAR,

  BLOCK_NUM
} block_e;

typedef enum {
  ROOM_NONE,
  ROOM_DINING,
  ROOM_THRONE,
  ROOM_LIBRARY,
  ROOM_STORAGE,
  ROOM_ARMORY,
  ROOM_HALLWAY,

  ROOM_NUM
} room_e;

static int get_walkable(int tile) {
  switch (tile) {
    case BLOCK_NONE:
    case BLOCK_WATER_DEEP:
    case BLOCK_WALL_V:
    case BLOCK_WALL: {
      return 0;
    }
  }
  
  return 1;
}

static int get_solid(int tile) {
  switch (tile) {
    case BLOCK_NONE:
    case BLOCK_WALL_V:
    case BLOCK_DOOR:
    case BLOCK_WALL: {
      return 0;
    }
  }
  
  return 1;
}

void db();

void db_set(int item);

#endif // DB_H