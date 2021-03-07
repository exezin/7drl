#ifndef DB_H
#define DB_H

// default values
#define WINDOW_TITLE  "rl"
#define WINDOW_WIDTH  432
#define WINDOW_HEIGHT 261
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

#define PROJECTILE_SPEED 0.01f;

typedef enum {
  IDENT_UNKNOWN,
  IDENT_PLAYER,
  IDENT_NPC,
  IDENT_CONTAINER,

  IDENT_NUM
} ident_e;

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
    case BLOCK_WALL: {
      return 0;
    }
  }
  
  return 1;
}

#endif // DB_H