#ifndef GAME_H
#define GAME_H

#include "main.h"

typedef enum {
  TILE_TYPE_SOLID,
  TILE_TYPE_LAVA,

  TILE_TYPE_NUM
} TILE_TYPES_E;

typedef enum {
  TILE_RENDER_DIRT

} TILE_RENDER_E;

typedef struct {
  int from[2], to[2];
  int tile;
  u8 r, g, b;
} projectile_t;

extern projectile_t projectile;

extern double game_tick;
extern int path_to_player[], path_from_player[], path_to_mouse[];
extern int paused;
extern int tile_on;
extern int dungeon_depth;

void projectile_start(int fromx, int fromy, int tox, int toy, int t);

ERR game_init();

int game_run();

void game_update(double step, double dt);

void game_render();

void game_keypressed(SDL_Scancode key);

void game_mousepressed(int button);

void game_mousewheel(int dx, int dy);

void game_mousemotion(int dx, int dy);

#endif // GAME_H