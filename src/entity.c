#include "entity.h"
#include "game.h"
#include "render/render.h"

entity_t *entity_stack[ENTITY_STACK_MAX] = {0};

extern tilesheet_packet_t level, entity_tiles;

u8 level_alpha[TILES_NUM] = {0};
u8 fov_alpha[TILES_NUM] = {0};

void entity_new(entity_t **ret, u32 identifier)
{
  for (int i=0; i<ENTITY_STACK_MAX; i++) {
    if (entity_stack[i] == NULL) {
      entity_stack[i] = calloc(1, sizeof(entity_t));
      entity_stack[i]->alive = 1;
      entity_stack[i]->id    = i;
      entity_stack[i]->ident = identifier;
      *ret = entity_stack[i];
      return;
    }
  }

  *ret = NULL;
}

void entity_remove(u32 id)
{
  if (id < ENTITY_STACK_MAX && entity_stack[id]) {
    entity_t *e = entity_stack[id];

    if (e->components.position && e->components.renderable) {
      entity_tiles.tiles[to_index(e->position.to[0], e->position.to[1])].tile = 0;
    }

    free(entity_stack[id]);
    entity_stack[id] = NULL;
  }
}

entity_t *entity_get(int x, int y)
{
  for (int i=0; i<ENTITY_STACK_MAX; i++) {
    entity_t *e = entity_stack[i];

    if (!e || !e->alive || !e->components.position)
      return NULL;

    if (e->position.to[0] == x && e->position.to[1] == y)
      return e;
  }

  return NULL;
}

/*-----------------------------------------/
/---------------- MISC --------------------/
/-----------------------------------------*/
void dijkstra(int *arr, int tox, int toy, int w, int h)
{
  if (tox >= 0 && toy >= 0 ) {
    for (int y=0; y<h; y++) {
      for (int x=0; x<w; x++) {
        u32 index = (y * w) + x;
        int tile = level.tiles[index].tile;
        arr[index] = get_walkable(tile) ? DIJ_MAX : -(DIJ_MAX+1);

        if (x == tox && y == toy) {
          arr[index] = 0;
        }
      }
    }
  }

  int changed = 1;
  while (changed) {
    changed = 0;
    for (int y=0; y<h; y++) {
      for (int x=0; x<w; x++) {
        int index = (y*w)+x;
        int tile = arr[index];

        // make sure its a walkable tile
        if (tile == -(DIJ_MAX+1))
          continue;

        // find the lowest value surrounding tile
        int lowest=DIJ_MAX;
        for (int i=0; i<8; i++) {
          int tx = MAX(0, MIN((x + around[i][0]), w));
          int ty = MAX(0, MIN((y + around[i][1]), h));
          int ti = (ty*w)+tx;
          if (tx == x && ty == y)
            continue;

          // make sure its walkable
          int value = arr[ti];
          if (value == -(DIJ_MAX+1))
            continue;

          // is it the lowest value tile?
          if (value < lowest)
            lowest = value;
        }

        if (lowest < DIJ_MAX && arr[index] > lowest+1) {
          arr[index] = lowest+1;
          changed++;
        }
      }
    }
  }
}

int dijkstra_lowest(vec2 out, int *arr, int tilex, int tiley, int w, int h)
{
  int on = arr[(tiley * w) + tilex];
  
  int lowest = DIJ_MAX;
  for (int i=0; i<8; i++) {
    int x = MAX(0, MIN(tilex + around_adjacent[i][0], w));
    int y = MAX(0, MIN(tiley + around_adjacent[i][1], h));
    int tile = arr[(y * w) + x];
    
    // move to it
    if (tile > -(DIJ_MAX+1) && tile < lowest && tile < on) {
      lowest = tile;
      out[0] = x; out[1] = y;
    }
  }

  return lowest;
}

void fov(entity_t *e)
{
  for (int i=0; i<TILES_NUM; i++)
    level.tiles[i].a = level_alpha[i];

  int distance = 10;
  for (double f = 0; f < 3.14*2; f += 0.01) {
    float fromx = e->position.to[0];
    float fromy = e->position.to[1];

    float tox = CLAMP(fromx + 0.5f + ((float)distance * cos(f)), 0, level.w-1);
    float toy = CLAMP(fromy + 0.5f + ((float)distance * sin(f)), 0, level.h-1);

    int x = (int)fromx, y = (int)fromy, done = 0;
    while (!done) {
      done = line(&x, &y, (int)tox, (int)toy);
      x = CLAMP(x, 0, level.w-1);
      y = CLAMP(y, 0, level.h-1);

      tile_t *tile = &level.tiles[(y*level.w)+x];
      int d = hypot(fromx - x, fromy - y);
      u8 alpha = CLAMP(255 - (distance * d * 2), 0, 255);
      tile->a = alpha;
      fov_alpha[(y*level.w)+x] = alpha;
      level_alpha[(y*level.w)+x] = 50.0f;

      for (int j=0; j<4; j++) {
        int tx = CLAMP(abs(x + adjacent[j][0]), 0, level.w-1);
        int ty = CLAMP(abs(y + adjacent[j][1]), 0, level.h-1);
        level.tiles[(ty * level.w) + tx].a = alpha;
        fov_alpha[(ty * level.w) + tx] = alpha;
        level_alpha[(ty * level.w) + tx] = 50.0f;
      }

      if (!get_solid(tile->tile) || tile->tile == BLOCK_DOOR) {
        break;
      }
    }
  }
}

int inventory_add(entity_t *e, int item, int uses)
{
  for (int i=0; i<INVENTORY_MAX; i++) {
    if (e->inventory.items[i] == ITEM_NONE) {
      e->inventory.items[i] = item;
      e->inventory.uses[i] = uses;
      return 1;
    }
  }

  return 0;
}


/*-----------------------------------------/
/---------------- SYSTEMS -----------------/
/-----------------------------------------*/
void system_renderable(entity_t *e)
{
  if (!e->components.renderable || !e->components.position)
    return;

  entity_tiles.tiles[to_index(e->position.from[0], e->position.from[1])].tile = 0;

  if (!e->alive)
    return;
  
  tile_t *tile = &entity_tiles.tiles[to_index(e->position.to[0], e->position.to[1])];
  tile->tile = e->renderable.tile;
  tile->r    = e->renderable.rgba[0];
  tile->g    = e->renderable.rgba[1];
  tile->b    = e->renderable.rgba[2];
  tile->a    = level.tiles[to_index(e->position.to[0], e->position.to[1])].a;//e->renderable.rgba[3];
  if (tile->a <= 50.0f)
    tile->a = 0.0f;

  e->position.from[0] = e->position.to[0];
  e->position.from[1] = e->position.to[1];
}

void system_move(entity_t *e)
{
  /*-----------------------------------------/
  figure out where we want to move to
  /-----------------------------------------*/
  if (!e->components.position || !e->components.move)
    return;

  // do we have energy to move?
  if (e->energy < ENERGY_MIN)
    return;

  // get tile to move to
  vec2 to = {e->position.to[0], e->position.to[1]};
  u32 tile = 0;

  // dont pathfind if its one tile away
  if ((to[0] != e->move.target[0] || to[1] != e->move.target[1]) && !e->move.dmap) {
    to[0] = e->move.target[0]; to[1] = e->move.target[1];
  } else if (e->move.dmap) {
    vec2 out;
    int d = dijkstra_lowest(out, e->move.dmap, e->position.to[0], e->position.to[1], e->move.w, e->move.h);
    if (d < DIJ_MAX) {
      to[0] = out[0];
      to[1] = out[1];
    }

    if (to[0] == e->position.to[0] && to[1] == e->position.to[1]) {
      action_stop(e);
      return;
    }
  } else {
    return;
  }

  /*-----------------------------------------/
  Do the movement, and specific actions based on what
  we attempt to move onto
  /-----------------------------------------*/
  // do entity-based action
  int entity_tile = entity_tiles.tiles[to_index(to[0], to[1])].tile;
  entity_t *entity = NULL;
  if (entity_tile)
    entity = entity_get(to[0], to[1]);
  u32 ident = (entity != NULL) ? entity->ident : IDENT_UNKNOWN;
  if (entity && e->id == entity->id)
    ident = IDENT_UNKNOWN;
  switch (ident) {
    // do bump attack
    case IDENT_PLAYER:
    case IDENT_NPC: {
      action_bump(e, entity);
      action_stop(e);
      e->energy -= ENERGY_MIN;
      return;
    }
    case IDENT_UNKNOWN: {
      break;
    }
  }

  // do tile-based action
  tile = level.tiles[to_index(to[0], to[1])].tile;
  switch (tile) {
    // hit a solid, perform no action
    case BLOCK_WALL_V:
    case BLOCK_WALL: {
      action_stop(e);

      if (e->ident == IDENT_PLAYER)
        paused = 1;
      break;
    }

    // open door
    case BLOCK_DOOR: {
      action_open(e, to[0], to[1]);
      action_stop(e);
      break;
    }

    // do move
    default: {
      e->position.to[0] = to[0];
      e->position.to[1] = to[1];
      e->energy -= ENERGY_MIN;

      // are we the player?
      if (e->ident == IDENT_PLAYER) {
        dijkstra(path_to_player, to[0], to[1], TILES_X, TILES_Y);

        // flee map
        memcpy(path_from_player, path_to_player, sizeof(int) * TILES_X * TILES_Y);
        for (int i=0; i<TILES_NUM; i++) {
        if (path_from_player[i] == DIJ_MAX)
            path_from_player[i] = -DIJ_MAX;
          else if (path_from_player[i] != -(DIJ_MAX+1))
            path_from_player[i] = -(path_to_player[i] * 1.2f);
        }
        dijkstra(path_from_player, -1, -1, TILES_X, TILES_Y);

        fov(e);
      }
      break;
    }
  }
}

void system_energy(entity_t *e)
{
  e->energy += e->speed.speed;
  // if (e->energy > ENERGY_MIN)
    // e->energy = ENERGY_MIN;
}

void system_stats(entity_t *e)
{
  if (!e->components.stats)
    return;
  
  if (e->stats.health <= 0)
    e->alive = 0;


  // handle death
  if (!e->alive) {
    
  }
}

void system_inventory(entity_t *e)
{

}


/*-----------------------------------------/
/---------------- ACTIONS -----------------/
/-----------------------------------------*/
void action_move(entity_t *e, u32 x, u32 y)
{
  if (!e->components.move)
    return;

  e->move.dmap = NULL;
  e->move.target[0] = x;
  e->move.target[1] = y;
}

void action_path(entity_t *e, int *path, u32 w, u32 h)
{
  if (!e->components.move)
    return;

  e->move.dmap = path;
  e->move.w = w;
  e->move.h = h;
}

void action_stop(entity_t *e)
{
  if (!e->components.move || !e->components.position)
    return;

  e->move.target[0] = e->position.to[0];
  e->move.target[1] = e->position.to[1];
  e->move.dmap = NULL;
}

void action_open(entity_t *e, u32 x, u32 y)
{
  // do we have energy to move?
  if (e->energy < ENERGY_MIN)
    return;

  int distance = MAX(abs(e->position.to[0] - x), abs(e->position.to[1] - y));
  if (distance > 1)
    return;

  tile_t *tile = &level.tiles[to_index(x, y)];
  switch (tile->tile) {
    case BLOCK_DOOR: {
      tile->tile = BLOCK_DOOR_OPEN;
      e->energy -= ENERGY_MIN;
      break;
    }
    case BLOCK_DOOR_OPEN: {
      tile->tile = BLOCK_DOOR;
      e->energy -= ENERGY_MIN;
      break;
    }
  }
}

void action_bump(entity_t *a, entity_t *b)
{
  if (!a->components.stats || !b->components.stats)
    return;

  int damage = a->stats.base_damage;

  action_damage(b, damage);
}

void action_damage(entity_t *e, int damage)
{
  e->stats.health -= damage;

  // dead
  if (e->stats.health <= 0) {
    e->alive = 0;
    system_renderable(e);
  }

  if (e->ident == IDENT_PLAYER) {
    paused = 1;
    action_stop(e);
  }
}

void action_use(entity_t *e, int item)
{
  e->inventory.use = item;
}