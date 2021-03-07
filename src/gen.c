#include "gen.h"
#include "render.h"

static int max_width = 0;
static int max_height = 0;

/*
  How to solve the map generating long dead-ended paths:
  Define a maximum area for the map, try to place a room (only try a max of say, 100 times)
  and if that fails, regenerate a room and try again, have some limit so you dont get stuck
  trying to place rooms once the entire map is crammed full of rooms.

  This should retain the level to a more square shape, with lots of looping paths etc.

  Door ideas:
  Once the map is generated, find all wall tiles with a floor each side, if the pathing distance
  from one side to the other is long (say, you have to walk a long way around to get to the other side of the wall),
  then you punch a door out in that wall at the roll of a dice
*/

void print_slice(slice_t *map)
{
  for (int y=0; y<map->height; y++) {
    for (int x=0; x<map->width; x++) {
      u32 index = (y * map->width) + x;
      tile_data_t *tile  = &map->tiles[index];
      block_e    block   = tile->block;
      room_e     room    = tile->room;
      u32   room_id = tile->room_id;

      char p = ' ';

      switch (block) {
        case BLOCK_NONE: {
          p = ' ';
          break;
        }
        case BLOCK_WALL: {
          p = '#';
          break;
        }
        case BLOCK_FLOOR: {
          p = '.';
          break;
        }
        case BLOCK_HOLE: {
          p = '>';
          break;
        }
        case BLOCK_LAVA: {
          p = '!';
          break;
        }
        case BLOCK_WATER: {
          p = '~';
          break;
        }
        case BLOCK_PILLAR: {
          p = '|';
          break;
        }
        default: {
          break;
        }
      }

      printf("%c", p);
    }
    printf(" %i\n", y);
  }
}

void place_circle(slice_t *slice, u32 x, u32 y, float radius)
{
  // prefers radius to be a power of 2
  float center_x = x + radius + 0.5f;
  float center_y = y + radius + 0.5f;

  for (int i=0; i<360; i++) {
    // set floor
    for (int j=0; j<(int)radius; j++) {
      float dx = center_x + cos((float)i) * (float)j;
      float dy = center_y + sin((float)i) * (float)j;
      get_tile(slice, (int)dx, (int)dy)->block = BLOCK_FLOOR;
    }

    // set wall
    float dx = center_x + cos((float)i) * radius;
    float dy = center_y + sin((float)i) * radius;
    get_tile(slice, (int)dx, (int)dy)->block = BLOCK_WALL;
  }
}

void place_box(slice_t *slice, u32 x, u32 y, u32 w, u32 h)
{
  for (int iy=y; iy<y+h; iy++) {
    for (int ix=x; ix<x+w; ix++) {
      if (iy == y || iy == y+h-1)
        get_tile(slice, ix, iy)->block = BLOCK_WALL;
      else if (ix == x || ix == x+w-1)
        get_tile(slice, ix, iy)->block = BLOCK_WALL;
      else
        get_tile(slice, ix, iy)->block = BLOCK_FLOOR;
    }
  }
}

void place_cave(slice_t *slice, u32 size)
{
  size = MAX(size, 12);

  // prefers radius to be a power of 2
  float center_x = size/2 + 0.5f;
  float center_y = size/2 + 0.5f;
  int radius = size / 2;

  for (int i=0; i<360*2; i++) {
    if (!(rand() % size/4))
      continue;
    for (int j=0; j<radius; j++) {
      if (!(rand() % size/6))
        break;
      float dx = center_x + cos((float)i/2.0f) * (float)j;
      float dy = center_y + sin((float)i/2.0f) * (float)j;
      get_tile(slice, (int)dx, (int)dy)->block = BLOCK_FLOOR;
    }
  }

  for (int i=0; i<16; i++) {
    for (int y=0; y<size; y++) {
      for (int x=0; x<size; x++) {
        int alive = 0;
        for (int j=y-1; j<=y+1; j++) {
          for (int k=x-1; k<=x+1; k++) {
            if (get_tile_block(slice, k, j) == BLOCK_FLOOR)
              alive++;
          }
        }

        if (alive < 5)
          get_tile(slice, x, y)->block = BLOCK_NONE;
        else
          get_tile(slice, x, y)->block = BLOCK_FLOOR;

      }
    }
  }
}

void place_doors(slice_t *slice, u32 chance)
{
  for (int y=0; y<slice->height; y++) {
    for (int x=0; x<slice->width; x++) {
      if (get_tile(slice, x, y)->block != BLOCK_FLOOR)
        continue;

      // make sure we arent already near a door
      int found_door = 0;
      for (int j=y-2; j<=y+2; j++) {
        for (int k=x-2; k<=x+2; k++) {
          if (get_tile(slice, k, j)->block == BLOCK_DOOR) {
            found_door = 1;
          }
        }
      }

      if (found_door || (rand() % chance))
        continue;

      u32 floor_a[] = {
        x-1, y,
        x+1, y,
        x,   y-1,
        x,   y+1
      };

      u32 floor_b[] = {
        x+1, y,
        x-1, y,
        x,   y+1,
        x,   y-1
      };

      u32 wall_a[] = {
        x,   y+1,
        x,   y-1,
        x+1, y,
        x-1, y
      };

      u32 wall_b[] = {
        x,   y-1,
        x,   y+1,
        x-1, y,
        x+1, y
      };

      int placed = 0;
      for (int i=0; i<4; i++) {
        int fx_a = floor_a[(i*2)+0]; int fy_a = floor_a[(i*2)+1];
        int fx_b = floor_b[(i*2)+0]; int fy_b = floor_b[(i*2)+1];
        int wx_a = wall_a[(i*2)+0]; int wy_a = wall_a[(i*2)+1];
        int wx_b = wall_b[(i*2)+0]; int wy_b = wall_b[(i*2)+1];
        if (get_tile(slice, fx_a, fy_a)->block == BLOCK_FLOOR &&
            get_tile(slice, fx_b, fy_b)->block == BLOCK_FLOOR &&
            get_tile(slice, wx_a, wy_a)->block == BLOCK_WALL &&
            get_tile(slice, wx_b, wy_b)->block == BLOCK_WALL) {
          get_tile(slice, x, y)->block   = BLOCK_DOOR;
          placed = 1;
          break;
        }
      }
    }
  }
}

void place_halls(slice_t *slice, u32 chance)
{
  for (int y=1; y<slice->height-1; y++) {
    for (int x=1; x<slice->width-1; x++) {
      if (get_tile(slice, x, y)->block != BLOCK_WALL)
        continue;

      int found_door = 0;
      for (int j=y-3; j<=y+3; j++) {
        for (int k=x-3; k<=x+3; k++) {
          if (get_tile(slice, k, j)->block == BLOCK_DOOR) {
            found_door = 1;
          }
        }
      }
      if (found_door)
        continue;

      // make sure we arent already near a door
      int ground_tile[2] = {0}, empty_tile[2] = {0};
      int ground = 0, wall = 0, empty = 0;
      for (int i=0; i<4; i++) {
        int tx = MAX(0, MIN(x + adjacent[i][0], slice->width));
        int ty = MAX(0, MIN(y + adjacent[i][1], slice->height));
        if (get_tile(slice, tx, ty)->block == BLOCK_FLOOR) {
          ground++;
          ground_tile[0] = tx; ground_tile[1] = ty;
          continue;
        }
        if (get_tile(slice, tx, ty)->block == BLOCK_WALL) {
          wall++;
          continue;
        }
        if (get_tile(slice, tx, ty)->block == BLOCK_NONE) {
          empty++;
          empty_tile[0] = tx; empty_tile[1] = ty;
          continue;
        }
      }

      if (ground != 1 || empty != 1)
        continue;

      int dx = CLAMP(empty_tile[0] - ground_tile[0], -1, 1);
      int dy = CLAMP(empty_tile[1] - ground_tile[1], -1, 1);

      if (dx != 0 && dy != 0) {
        if (!(rand() % 2))
          dx = 0;
        else
          dy = 0;
      }

      int x1 = x, y1 = y;
      int path = 0;
      for (int i=0; i<10; i++) {
        x1 += dx;
        y1 += dy;

        int found_door = 0;
        for (int j=y1-1; j<=y1+1; j++) {
          for (int k=x1-1; k<=x1+1; k++) {
            if (get_tile(slice, k, j)->block == BLOCK_DOOR) {
              found_door = 1;
            }
          }
        }
        if (found_door)
          break;

        if (!i)
          continue;

        if (x1 <= 0 || y1 <= 0 || x1 >= slice->width || y1 >= slice->height)
          break;

        int walls = 0;
        for (int j=0; j<4; j++) {
          int tx = MAX(0, MIN(x1 + adjacent[j][0], slice->width));
          int ty = MAX(0, MIN(y1 + adjacent[j][1], slice->height));
          if (get_tile(slice, tx, ty)->block == BLOCK_WALL)
            walls++;
        }

        if (get_tile(slice, x1, y1)->block == BLOCK_FLOOR) {
          path = 1;
          break;
        } else if (get_tile(slice, x1, y1)->block == BLOCK_WALL) {
          if (get_tile(slice, x1+dx, y1+dy)->block != BLOCK_FLOOR)
            break;
        }
      }

      if (path) {
        int x1 = x, y1 = y;
        for (int i=0; i<10; i++) {
          if (get_tile(slice, x1, y1)->block == BLOCK_FLOOR) {
            get_tile(slice, x1, y1)->block = BLOCK_FLOOR;
            break;
          }
          get_tile(slice, x1, y1)->block = BLOCK_FLOOR;
          get_tile(slice, x1+dy, y1+dx)->block = BLOCK_WALL;
          get_tile(slice, x1-dy, y1-dx)->block = BLOCK_WALL;

          if (!i)
            get_tile(slice, x1, y1)->block = BLOCK_DOOR;

          x1 += dx;
          y1 += dy;
        }
      }
    }
  }
}

void clean_dungeon(slice_t *slice)
{
  for (int y=1; y<slice->height-1; y++) {
    for (int x=1; x<slice->width-1; x++) {
      if (get_tile(slice, x, y)->block != BLOCK_WALL)
        continue;

      int walls = 0, floors = 0;
      for (int j=0; j<8; j++) {
        int tx = MAX(0, MIN(x + around[j][0], slice->width));
        int ty = MAX(0, MIN(y + around[j][1], slice->height));
        if (tx == x && ty == y)
          continue;
        int tile = get_tile(slice, tx, ty)->block;
        if (tile == BLOCK_WALL || tile == BLOCK_NONE)
          walls++;
        if (tile == BLOCK_FLOOR)
          floors++;
      }
      if (walls == 8)
        get_tile(slice, x, y)->block = BLOCK_NONE;
    }
  }
}

void place_lake(slice_t *slice)
{
  int largest = 0, id = 0;
  for (int i=1; i<slice->room_count; i++) {
    int size = 0;
    for (int j=0; j<slice->width*slice->height; j++) {
      if (slice->tiles[j].room_id == i)
        size++;
    }

    if (size > largest) {
      largest = size;
      id = i;
    }
  }

  if (!id)
    return;

  for (int y=1; y<slice->height-1; y++) {
    for (int x=1; x<slice->width-1; x++) {
      tile_data_t *tile = get_tile(slice, x, y);
      if (tile->block != BLOCK_FLOOR || tile->room_id != id)
        continue;

      int walls = 0, floors = 0;
      for (int j=0; j<8; j++) {
        int tx = MAX(0, MIN(x + around[j][0], slice->width));
        int ty = MAX(0, MIN(y + around[j][1], slice->height));
        if (tx == x && ty == y)
          continue;
        int tile = get_tile(slice, tx, ty)->block;
        if (tile == BLOCK_WALL || tile == BLOCK_NONE)
          walls++;
        if (tile == BLOCK_FLOOR)
          floors++;
      }

      if (walls)
        continue;

      get_tile(slice, x, y)->block = BLOCK_WATER;
    }
  }

  for (int y=1; y<slice->height-1; y++) {
    for (int x=1; x<slice->width-1; x++) {
      tile_data_t *tile = get_tile(slice, x, y);
      if (tile->block != BLOCK_WATER)
        continue;

      int walls = 0;
      for (int j=y-2; j<=y+2; j++) {
        for (int k=x-2; k<=x+2; k++) {
          if (get_tile(slice, k, j)->block == BLOCK_WALL) {
            walls++;
          }
        }
      }

      int water = 0;
      for (int i=0; i<8; i++) {
        int tx = MAX(0, MIN(x + around[i][0], slice->width));
        int ty = MAX(0, MIN(y + around[i][1], slice->height));
        int tile = get_tile(slice, tx, ty)->block;
        if (tile == BLOCK_WATER || tile == BLOCK_WATER_DEEP)
          water++;
      }

      if ((walls > 1 && !(rand() % 4)) || water <= 2)
        tile->block = BLOCK_FLOOR;
      if (water >= 8)
        tile->block = BLOCK_WATER_DEEP;
    }
  }

  P_DBG("Largest room %i of size %i\n", id, largest);
}

void gen(tilesheet_packet_t *packet)
{
  // create initial empty map
  max_width = WINDOW_WIDTH / TILE_RWIDTH;
  max_height = WINDOW_HEIGHT / TILE_RHEIGHT;
  slice_t map = new_slice_sized(max_width, max_height);

  // initial room (placed in center)
  slice_t slice = new_slice();
  place_cave(&slice, 18 + rand() % 9);
  clean_slice(&slice);
  compress_slice(&slice);
  slice_set_id(&slice, map.room_count++);
  place_slice(&map, &slice);
  destroy_slice(&slice);

  for (int i=0; i<200 + rand() % 128; i++) {
    slice_t room_parts = new_slice();
    slice = new_slice();
    for (int i=0; i<2; i++) {
      if (!(rand() % 5)) {
        place_circle(&slice, 0, 0, 3 + rand() % 2);
        clean_slice(&slice);
        compress_slice(&slice);
        place_slice(&room_parts, &slice);
        destroy_slice(&slice);
        slice = new_slice();
      }
    }
    for (int i=0; i<3; i++) {
      if (!(rand() % 2)) {
        int size = 4 + rand() % 2;
        place_box(&slice, 0, 0, size + (rand() % 2), size + (rand() % 2));
        place_box(&slice, 0, 0, size + (rand() % 3), size + (rand() % 3));
        clean_slice(&slice);
        compress_slice(&slice);
        place_slice(&room_parts, &slice);
        destroy_slice(&slice);
        slice = new_slice();
      }
    }
    if (!(rand() % 3)) {
      destroy_slice(&room_parts);
      slice_t room_parts = new_slice();
      place_cave(&slice, 12 + rand() % 5);
      clean_slice(&slice);
      compress_slice(&slice);
      place_slice(&room_parts, &slice);
      destroy_slice(&slice);
      slice = new_slice();
    }

    clean_slice(&room_parts);
    compress_slice(&room_parts);
    place_slice(&map, &room_parts);
    destroy_slice(&room_parts);
    destroy_slice(&slice);
  }
  place_doors(&map, 1);
  place_halls(&map, 1);
  clean_dungeon(&map);
  place_lake(&map);

  // generate tilemap packet
  packet->x = 0, packet->y = 0;
  packet->zoom = 1;
  packet->w = (WINDOW_WIDTH / TILE_RWIDTH), packet->h = (WINDOW_HEIGHT / TILE_RHEIGHT);
  packet->rx = 0, packet->rw = packet->w;
  packet->ry = 0, packet->rh = packet->h;
  if (packet->tiles)
    free(packet->tiles);
  packet->tiles = calloc(1, sizeof(tile_t) * packet->w * packet->h);
  for (int y=0; y<map.height; y++) {
    for (int x=0; x<map.width; x++) {
      u32 i = (y * packet->w) + x;
      packet->tiles[i].tile = map.tiles[i].block;
      if (packet->tiles[i].tile == BLOCK_FLOOR && rand() % 100 < 5)
        packet->tiles[i].tile++;
      switch (packet->tiles[i].tile) {
        case BLOCK_WALL: {
          if (get_tile(&map, x, y+1)->block == BLOCK_WALL)
            packet->tiles[i].tile++;
          break;
        }
      }
      packet->tiles[i].r = 100 + (rand() % 80);
      packet->tiles[i].g = 100 + (rand() % 80);
      packet->tiles[i].b = 100 + (rand() % 80);
      if (packet->tiles[i].tile == BLOCK_FLOOR) {
        packet->tiles[i].r += 50;
        packet->tiles[i].g += 50;
        packet->tiles[i].b += 50;
      }
      if (packet->tiles[i].tile == BLOCK_WATER) {
        packet->tiles[i].r = 120;
        packet->tiles[i].g = 120;
        packet->tiles[i].b = 255;
      }
      if (packet->tiles[i].tile == BLOCK_WATER_DEEP) {
        packet->tiles[i].r = 50;
        packet->tiles[i].g = 50;
        packet->tiles[i].b = 150;
      }
      packet->tiles[i].a = 255;
    }
  }
  print_slice(&map);

  free(map.tiles);
}
