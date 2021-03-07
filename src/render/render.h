#ifndef RENDER_H
#define RENDER_H

#include "main.h"
#include "math/linmath.h"
#include "db.h"

typedef struct {
  float position[3];
  float uv[2];
  float normal[3];
  float tangent[4];
  u8 color[4];
} vertex_t;

typedef struct {
  GLuint vao, vbo, ebo, vcount, icount;
  GLuint texture, texture_spec, texture_norm;
} mesh_t;

typedef struct {
  u16 tile;
  u8 r,g,b,a;
} tile_t;

typedef struct {
  tile_t *tiles;
  float zoom;
  size_t w, h; // width and height of tilemap
  int x, y; // render centered on x and y
  int rx, ry; // render to screen at rx and ry
  int rw, rh; // render area size on screen
} tilesheet_packet_t;

extern tilesheet_packet_t packet;

ERR render_init();

void render_render();

int render_update();

void render_clean();

void render_tilemap(tilesheet_packet_t *packet);

// helper getters
static inline u32 window_width() {
  return (u32)ini_get_float(conf, "graphics", "window_width");
}

static inline u32 window_height() {
  return (u32)ini_get_float(conf, "graphics", "window_height");
}

static inline u32 to_index(u32 x, u32 y)
{
  x = CLAMP(x, 0, TILES_X);
  y = CLAMP(y, 0, TILES_Y);
  return (y * TILES_X) + x; 
}

void render_translate_mouse(int *mx, int *my);

#endif // RENDER_H