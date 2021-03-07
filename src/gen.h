/* gen.h

*/
#ifndef GEN_H
#define GEN_H

#include "main.h"
#include "types.h"
#include "db.h"
#include "math/linmath.h"
#include "render/render.h"

/*
  Things the generator will need:
  - Generate rooms of varying size, connect them to the
    rest of the map via a given door point, or multiple doors
  - Fill in rooms with related objects, bookshelves, foliage, chests,
    enemy spawns, literally everything the game level will need
  - Minimum spanning tree to remove certain room connections and make
    exploration a little bit more essential
  - Prefabs that can be connected to the map at specific door points,
    including partial prefabs that have random door locations or
    entity/storage/cosmetic spawns
  - Needs to specify what each block is in a general sense, like a wall
    block or a floor/hole/water block, should also store more specific data
    about the blocks, such as type of block the renderer should use, stone, marble,
    wood bookshelf, etc

  Restrictions:
  - Hallways need to be two blocks wide, maybe 3 at most
  - Rooms shouldnt be smaller than 4x4 tiles, average 5x5-10x10 tiles
  - Larger rooms are > 10x10 tiles
  - Doors are connected to the rooms walls, not the hallway
  - THIS:
    *****
    *   *######
    *   /     #
    *   *#### #
    *****   # #
  - NOT THIS:
    *****
    *   *######
    *    /    #
    *   *#### #
    *****   # #

  Terminology:
  - Slice, a small section separate from the main map to eventually be merged into it
  - Map, the main map, everything gets added to this eventually

  Functions the generator will need:
  - Generate room (one function per type, normal room, cavern, storeroom etc)
  - Generate hallway
  - Connect room/hallway to main map at given point (door positions)
    Hallways dont have doors of their own, they connect to doors of other rooms
  - Set tile at position (helper, for main map and slice)
  - Get tile at position (helper, for main map and slice)

  How to generate a room:
  - Generate rooms walls
  - Fill in room with stuff
  - If first room, place in center of map
  - If not first room, place on map where it connects to some other room or hallway
  - Punch out door between the newly placed room and the one its connected to
    Always punch door out on the side of the old room, not the newly placed one

  Generation order:
  - 1: Generate room
  - 2: Place on map in center
  - 3: Generate room OR generate hallway (higher chance of room?)
  - 4: Place room or hallway on map, at location where door connects to other parts of map
  - 5: Punch out doorway between newly placed room and old one
  - 6: Repeat 3-5 until map is filled
  - 7: Punch out doorways extra doorways where possible (at random)

  Meta-data the generator needs to know:
  - Type of level being generated, stone/mayble castle, swamp, lava pits etc
  - Amount of players (to define difficulty/size?)
  - Player depth, to define lighting conditions and difficulty

  Level data structure needs to contain:
  - General block type (Walls, floors, holes, bookshelves etc)
  - Specific block material (Walls, floors, holes, bookshelves etc)
  - Room type
  - Unique room ID
  - Entity spawns (items, chests, enemy spawns etc)
  - Secondary entity spawns (candles, weapons, stuff that can spawn on other things)
  - Cosmetic spawns (non-collidables, vines, grass etc)

  Things that ARE an entity:
  - Enemies
  - Interactables (Chests, shrines, doors etc)
  - Static objects that ARENT blocks, bookshelves etc
  - Basically anything that would go ontop of a floor tile

  Ideas:
  - Room prefabs
  - Room entity prefabs (lists of entities that should spawn together in a room
    like goblins with a treasure chest, a monster in a cage, corpses and items
    or a dining table with chairs, prefabs like this could have a space requirement)
    room and entity prefabs would have associated room types that they can spawn in
    for example, table and chairs in dining all, weapons and orcs around a chest in a
    armory etc
    Prefabs should have a min/max sized area they can spawn in, and a flag to decide
    if multiple prefabs can spawn per room or not
  - Monsters spawn out of line of sight to the spawn to prevent spawn ambush
  - Monsters, weapons etc spawn with random modifiers/mutations

 */

typedef struct {
  block_e    block;    // general block type
  room_e     room;     // room type
  u32        room_id;  // unique room ID
} tile_data_t;

void gen(tilesheet_packet_t *packet);

#define SLICE_SIZE 128

typedef struct {
  tile_data_t *tiles;
  u32 width, height;
  u32 room_count;
} slice_t;

static inline slice_t new_slice() {
  slice_t slice;
  slice.width  = SLICE_SIZE;
  slice.height = SLICE_SIZE;
  slice.room_count = 1;
  slice.tiles  = malloc(sizeof(tile_data_t) * SLICE_SIZE * SLICE_SIZE);
  memset(slice.tiles, 0, sizeof(tile_data_t) * SLICE_SIZE * SLICE_SIZE);
  for (int i=0; i<SLICE_SIZE*SLICE_SIZE; i++)
    slice.tiles[i].block = BLOCK_NONE;
  return slice;
}

static inline slice_t new_slice_sized(u32 width, u32 height) {
  slice_t slice;
  slice.width  = width;
  slice.height = height;
  slice.room_count = 1;
  slice.tiles  = calloc(1, sizeof(tile_data_t) * width * height);
  return slice;
}

static inline void destroy_slice(slice_t *slice) {
  free(slice->tiles);
}

static inline block_e get_tile_block(slice_t *slice, u32 x, u32 y) {
  if (x < 0 || x >= slice->width)
    return BLOCK_NONE;
  if (y < 0 || y >= slice->height)
    return BLOCK_NONE;
  u32 index = (y * slice->width) + x;
  return slice->tiles[index].block;
}

static inline tile_data_t* get_tile(slice_t *slice, int x, int y) {
  x = CLAMP(abs(x), 0, slice->width-1);
  y = CLAMP(abs(y), 0, slice->height-1);
  u32 index = (y * slice->width) + x;
  return &slice->tiles[index];
}

static inline void clean_slice(slice_t *slice) {
  // remove all walls
  for (int i=0; i<slice->width*slice->height; i++) {
    if (slice->tiles[i].block == BLOCK_WALL)
      slice->tiles[i].block = BLOCK_FLOOR;
  }

  // replace walls
  for (int y=0; y<slice->height; y++) {
    for (int x=0; x<slice->width; x++) {
      if (get_tile(slice, x, y)->block != BLOCK_FLOOR)
        continue;

      for (int j=y-1; j<=y+1; j++) {
        for (int k=x-1; k<=x+1; k++) {
          if (get_tile_block(slice, k, j) == BLOCK_NONE)
            get_tile(slice, x, y)->block = BLOCK_WALL;
        }
      }
    }
  }
}

// ---
static inline void set_room(slice_t *slice, room_e room) {
  for (int i=0; i<slice->width*slice->height; i++) {
    if (slice->tiles[i].block == BLOCK_NONE)
      continue;

    slice->tiles[i].room     = room;
    slice->tiles[i].room_id  = slice->room_count;
  }

  slice->room_count++;
}
// ---

static inline void blit_slice(slice_t *map, slice_t *slice, u32 x, u32 y) {
  for (int iy=y; iy<y+slice->height; iy++) {
    for (int ix=x; ix<x+slice->width; ix++) {
      tile_data_t *t1 = get_tile(map, ix, iy);
      tile_data_t *t2 = get_tile(slice, ix-x, iy-y);
      if (t2->block == BLOCK_NONE)
        continue;
      *t1 = *t2;
    }
  }
}

static inline int blit_possible(slice_t *map, slice_t *slice, u32 x, u32 y) {
  for (int iy=y; iy<y+slice->height; iy++) {
    for (int ix=x; ix<x+slice->width; ix++) {
      if (x < 0 || x >= map->width || y < 0 || y >= map->height)
        return 0;

      if (ix < 0 || ix >= map->width || iy < 0 || iy >= map->height)
        return 0;

      tile_data_t *t1 = get_tile(map, ix, iy);
      tile_data_t *t2 = get_tile(slice, ix-x, iy-y);
      if ((t1->block != BLOCK_NONE && t2->block != BLOCK_NONE) && 
          (t1->block == BLOCK_WALL && t2->block == BLOCK_WALL))
        return 0;
    }
  }

  return 1;
}

static inline void slice_set_id(slice_t *slice, u32 id) {
  for (int i=0; i<slice->width*slice->height; i++)
    slice->tiles[i].room_id = id;
}

static inline int place_slice(slice_t *map, slice_t *slice) {
  typedef struct {
    u32 x, y;
    u32 ex, ey, fx, fy;
  } door_t;

  door_t *doors = malloc(sizeof(door_t) * map->width * map->height);
  size_t door_i = 0;

  // find all viable door tiles
  for (int y=0; y<map->height; y++) {
    for (int x=0; x<map->width; x++) {
      if (get_tile(map, x, y)->block != BLOCK_WALL)
        continue;

      u32 floors[] = {
        x-1, y,
        x+1, y,
        x,   y-1,
        x,   y+1
      };

      u32 empty[] = {
        x+1, y,
        x-1, y,
        x,   y+1,
        x,   y-1
      };

      for (int i=0; i<4; i++) {
        int fx = floors[(i*2)+0];
        int fy = floors[(i*2)+1];
        int ex = empty[(i*2)+0];
        int ey = empty[(i*2)+1];
        if (get_tile(map, fx, fy)->block == BLOCK_FLOOR &&
            get_tile(map, ex, ey)->block == BLOCK_NONE) {
          doors[door_i].x = x;
          doors[door_i].y = y;
          doors[door_i].ex = ex;
          doors[door_i].ey = ey;
          doors[door_i].fx = fx;
          doors[door_i].fy = fy;
          door_i++;
          break;
        }
      }
    }
  }

  // no usable doors? place in center
  if (!door_i) {
    blit_slice(map, slice, (map->width/2)-(slice->width/2), (map->height/2)-(slice->height/2));
    goto done;
  }

  // randomize viable door list
  for (int i=0; i<door_i; i++) {
    door_t temp_door = doors[i];
    size_t rand_index = rand() % door_i;
    doors[i] = doors[rand_index];
    doors[rand_index] = temp_door;
  }

  u32 width  = slice->width;
  u32 height = slice->height;

  // find usable door position
  for (int i=0; i<door_i; i++) {
    u32 from_x = doors[i].x - (slice->width);
    u32 from_y = doors[i].y - (slice->height);
    u32 to_x   = doors[i].x + slice->width;
    u32 to_y   = doors[i].y + slice->height;

    for (int y=from_y; y<to_y; y++) {
      for (int x=from_x; x<to_x; x++) {
        u32 door_x = doors[i].x;
        u32 door_y = doors[i].y;
        u32 dx = door_x + (doors[i].ex - doors[i].x);
        u32 dy = door_y + (doors[i].ey - doors[i].y);
        u32 rx = door_x + ((doors[i].ex - doors[i].x) * 2);
        u32 ry = door_y + ((doors[i].ey - doors[i].y) * 2);

        if (get_tile_block(slice, dx-x, dy-y) == BLOCK_WALL &&
            get_tile_block(slice, rx-x, ry-y) == BLOCK_FLOOR &&
            blit_possible(map, slice, x, y)) {
          // place slice on map
          slice_set_id(slice, map->room_count++);
          blit_slice(map, slice, x, y);
          get_tile(map, door_x, door_y)->block = BLOCK_FLOOR;
          get_tile(map, dx, dy)->block = BLOCK_FLOOR;
          goto done;
        }
      }
    }
  }

  free(doors);
  return 0;

  done:
  free(doors);
  return 1;
}

static inline void compress_slice(slice_t *map)
{
  // find the boundries of the map tiles
  u32 minx=9999, miny=9999, maxx=0, maxy=0;
  for (int y=0; y<map->height; y++) {
    for (int x=0; x<map->width; x++) {
      if (get_tile(map, x, y)->block == BLOCK_NONE)
        continue;

      if (x < minx)
        minx = x;
      if (y < miny)
        miny = y;
      if (x > maxx)
        maxx = x;
      if (y > maxy)
        maxy = y;
    }
  }

  if (minx == 9999 || miny == 9999 || maxx == 0 || maxy == 0)
    return;

  maxx++; maxy++;
  u32 width = maxx - minx, height = maxy - miny;
  tile_data_t *new_tiles = malloc(sizeof(tile_data_t) * width * height);

  // extract the section from the tile data
  for (int y=miny; y<maxy; y++) {
    for (int x=minx; x<maxx; x++) {
      tile_data_t *t1 = &new_tiles[((y - miny) * width) + (x - minx)];
      tile_data_t *t2 = get_tile(map, x, y);
      *t1 = *t2;
    }
  }

  // replace old tile data with newly extracted section
  free(map->tiles);
  map->tiles  = new_tiles;
  map->width  = width;
  map->height = height;
}

#endif // GEN_H