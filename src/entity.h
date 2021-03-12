#ifndef ENTITY_H
#define ENTITY_H

#define ENTITY_STACK_MAX 512

#include "main.h"
#include "types.h"
#include "db.h"
#include "render/render.h"
#include "math/linmath.h"

extern u8 level_alpha[TILES_NUM];
extern u8 fov_alpha[TILES_NUM];

typedef struct comp_ai_t {
  int aggro, target, hostile;
  int dumb, splitter, flees;
} comp_ai_t;

typedef struct comp_inventory_t {
  int items[INVENTORY_MAX];
  int uses[INVENTORY_MAX];
  int equipt[INVENTORY_MAX];
  int use, drop, fire, get;
  int fire_x, fire_y;
} comp_inventory_t;

typedef struct comp_container_t {
  int item, uses;
} comp_container_t;

typedef struct comp_stats_t {
  int health, max_health;
  int level, exp;
  int expmod;
  int base_damage;
} comp_stats_t;

typedef struct comp_position_t {
  int from[2], to[2];
} comp_position_t;

typedef struct comp_speed_t {
  float speed;
} comp_speed_t;

typedef struct comp_renderable_t {
  u32 tile;
  vec4 rgba;
} comp_renderable_t;

typedef struct comp_move_t {
  int target[2];
  int *dmap;
  int w, h;
} comp_move_t;

typedef struct {
  u32 position   : 1;
  u32 speed      : 1;
  u32 renderable : 1;
  u32 move       : 1;
  u32 stats      : 1;
  u32 inventory  : 1;
  u32 container  : 1;
  u32 ai         : 1;
} comp_flags_t;

typedef struct {
  u32 id, ident;
  int alive;
  float energy;
  char name[64], description[128];

  // components we can have
  comp_flags_t      components;
  
  // the components
  comp_position_t   position;
  comp_speed_t      speed;
  comp_renderable_t renderable;
  comp_move_t       move;
  comp_stats_t      stats;
  comp_inventory_t  inventory;
  comp_container_t  container;
  comp_ai_t         ai;
} entity_t;

extern entity_t *entity_stack[ENTITY_STACK_MAX];

// component initializers
static void comp_position(entity_t *e, u32 x, u32 y) {
  e->components.position = 1;
  e->position.from[0] = x; e->position.from[1] = y;
  e->position.to[0] = x; e->position.to[1] = y;
}
static void comp_renderable(entity_t *e, u32 tile, u8 r, u8 g, u8 b, u8 a) {
  e->components.renderable = 1;
  e->renderable.tile = tile;
  e->renderable.rgba[0] = r;
  e->renderable.rgba[1] = g;
  e->renderable.rgba[2] = b;
  e->renderable.rgba[3] = a;
}
static void comp_speed(entity_t *e, float speed) {
  e->components.speed = 1;
  e->speed.speed = speed;
}
static void comp_move(entity_t *e) {
  e->components.move = 1;
  e->move.target[0] = e->position.to[0];
  e->move.target[1] = e->position.to[1];
  e->move.dmap = NULL;
}
static void comp_stats(entity_t *e, int health, int level, int base_damage) {
  e->components.stats = 1;
  e->stats.health = health;
  e->stats.max_health = health;
  e->stats.level = level;
  e->stats.exp = 0;
  e->stats.base_damage = base_damage;
}
static void comp_inventory(entity_t *e)
{
  e->components.inventory = 1;
  memset(e->inventory.items, ITEM_NONE, sizeof(int) * INVENTORY_MAX);
  memset(e->inventory.uses, 0, sizeof(int) * INVENTORY_MAX);
  memset(e->inventory.equipt, 0, sizeof(int) * INVENTORY_MAX);
  e->inventory.use  = -1;
  e->inventory.drop = -1;
  e->inventory.fire = -1;
  e->inventory.get  = -1;
}
static void comp_container(entity_t *e, int item, int uses, int x, int y)
{
  int tile = 0;
  if (item > ITEM_POTION_START && item < ITEM_POTION_END)
    tile = BLOCK_POTION;
  if (item > ITEM_SCROLL_START && item < ITEM_SCROLL_END)
    tile = BLOCK_SCROLL;
  if (item > ITEM_GEAR_START && item < ITEM_GEAR_WEAPON_START)
    tile = BLOCK_GEAR;
  if (item > ITEM_GEAR_WEAPON_START && item < ITEM_GEAR_END)
    tile = BLOCK_SWORD;
  if (item > ITEM_WAND_START && item < ITEM_WAND_END)
    tile = BLOCK_WAND;
  if (item == ITEM_KEY)
    tile = 68+9;
  if (item == ITEM_NONE)
    return;
  comp_renderable(e, tile, 255, 120, 255, 255);
  comp_position(e, x, y);
  e->components.container = 1;
  e->container.item = item;
  e->container.uses = uses;
}
static void comp_ai(entity_t *e)
{
  e->components.ai = 1;
  memset(&e->ai, 0, sizeof(comp_ai_t));
  e->ai.target = -1;
}

void entity_new(entity_t **ret, u32 identifier, const char *name);
void entity_remove(u32 id);
entity_t *entity_get(int x, int y);


void dijkstra(int *arr, int tox, int toy, int w, int h);
int dijkstra_lowest(vec2 out, int *arr, int tilex, int tiley, int w, int h);
int inventory_add(entity_t *e, int item, int uses);
void player_path(entity_t *e);
void container(int item, int uses, int x, int y);
void fov(entity_t *e);

void goblin(int level, int x, int y);
void goblin_caster(int level, int x, int y);
void jackel(int level, int x, int y);
void zombie(int level, int x, int y);
void bat(int level, int x, int y);
void blob(int level, int x, int y, int angry);
void wizard(int level, int x, int y);

void system_move(entity_t *e);
void system_renderable(entity_t *e);
void system_energy(entity_t *e);
void system_stats(entity_t *e);
void system_inventory(entity_t *e);
void system_ai(entity_t *e);


void action_move(entity_t *e, u32 x, u32 y);
void action_path(entity_t *e, int *path, u32 w, u32 h);
void action_stop(entity_t *e);
void action_open(entity_t *e, u32 x, u32 y);
void action_bump(entity_t *a, entity_t *b);
void action_damage(entity_t *a, entity_t *b, int damage);
void action_use(entity_t *e, int item);
void action_drop(entity_t *e, int item);
void action_fire(entity_t *e, int item, int x, int y);
void action_get(entity_t *e);


#endif // ENTITY_H