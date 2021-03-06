#include "entity.h"
#include "game.h"
#include "ui.h"
#include "render/render.h"

entity_t *entity_stack[ENTITY_STACK_MAX] = {0};

extern tilesheet_packet_t level, entity_tiles;

u8 level_alpha[TILES_NUM] = {0};
u8 fov_alpha[TILES_NUM] = {0};

extern entity_t *player;

void entity_new(entity_t **ret, u32 identifier, const char *name)
{
  for (int i=0; i<ENTITY_STACK_MAX; i++) {
    if (entity_stack[i] == NULL) {
      entity_stack[i] = calloc(1, sizeof(entity_t));
      entity_stack[i]->alive = 1;
      entity_stack[i]->id    = i;
      entity_stack[i]->ident = identifier;
      entity_stack[i]->energy = 0.0f;
      strcpy(entity_stack[i]->name, name);
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

    free(entity_stack[id]);
    entity_stack[id] = NULL;
  }
}

entity_t *entity_get(int x, int y)
{
  for (int i=0; i<ENTITY_STACK_MAX; i++) {
    entity_t *e = entity_stack[i];

    if (!e || !e->alive || !e->components.position)
      continue;

    if (e->position.to[0] == x && e->position.to[1] == y)
      return e;
  }

  return NULL;
}

entity_t *entity_get_npc(int x, int y)
{
  for (int i=0; i<ENTITY_STACK_MAX; i++) {
    entity_t *e = entity_stack[i];

    if (!e || !e->alive || !e->components.position || (e->ident != IDENT_NPC && e->ident != IDENT_PLAYER))
      continue;

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
        entity_t *e = entity_get_npc(x, y);
        int ident = e ? e->ident : IDENT_UNKNOWN;
        if (ident == IDENT_PLAYER || ident == IDENT_NPC)
          arr[index] = -(DIJ_MAX+1);

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
  if (on == -(DIJ_MAX+1))
    on = DIJ_MAX;
  
  int lowest = DIJ_MAX;
  for (int i=0; i<8; i++) {
    int x = MAX(0, MIN(tilex + around_adjacent[i][0], w));
    int y = MAX(0, MIN(tiley + around_adjacent[i][1], h));
    int tile = arr[(y * w) + x];

    if (tile == -(DIJ_MAX+1))
      continue;
    
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

    int x = (int)fromx, y = (int)fromy, done = 0, distance = 0;
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

      if (!get_solid(tile->tile) || tile->tile == BLOCK_DOOR || distance > 50) {
        break;
      }

      distance++;
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

void player_path(entity_t *e)
{
  // are we the player?
  dijkstra(path_to_player, e->position.to[0], e->position.to[1], TILES_X, TILES_Y);

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

void container(int item, int uses, int x, int y)
{
  entity_t *e;
  entity_new(&e, IDENT_CONTAINER, "A BAG");
  comp_container(e, item, uses, x, y);
}

void setdesc(entity_t *e, const char *desc)
{
  strncpy(e->description, desc, MIN(strlen(desc)+1, 128));
}

void goblin(int level, int x, int y)
{
  entity_t *monster;
  entity_new(&monster, IDENT_NPC, "GOBLIN");
  comp_position(monster, x, y);
  comp_renderable(monster, 'G'-64, 120, 200, 120, 255);
  comp_speed(monster, 0.25f);
  comp_move(monster);
  comp_stats(monster, 80 + (5 * level), level, 2 + (3 * (level - 1)));
  comp_ai(monster);
  comp_inventory(monster);
  monster->ai.flees = 1;
  monster->stats.expmod = 1;
  if (level < 2) {
    inventory_add(monster, ITEM_GEAR_IRONDAGGER, 1);
  } else if (level < 4) {
    if (!(rand() % 2))
      inventory_add(monster, ITEM_GEAR_IRONDAGGER, 1);
    else
      inventory_add(monster, ITEM_GEAR_IRONSWORD, 1);
  } else {
    inventory_add(monster, ITEM_GEAR_GREATSWORD, 1);
  }

  //                ######################|######################|######################|######################|
  setdesc(monster, "A GOBLIN WARRIOR. SLOW BUT HITS HARD");
}

void goblin_caster(int level, int x, int y)
{
  entity_t *monster;
  entity_new(&monster, IDENT_NPC, "GOBLIN CASTER");
  comp_position(monster, x, y);
  comp_renderable(monster, 'G'-64, 120, 255, 200, 255);
  comp_speed(monster, 0.5f);
  comp_move(monster);
  comp_stats(monster, 40 + (5 * level), level, 2 + (1 * (level - 1)));
  comp_ai(monster);
  comp_inventory(monster);
  inventory_add(monster, ITEM_WAND_FIREBOLT, 10);
  monster->ai.flees = 1;
  monster->stats.expmod = 2;

  //                ######################|######################|######################|######################|
  setdesc(monster, "A GOBLIN MAGE. LIKELY  DEADLY AT RANGE BUT    RATHER FEEBLE");
}

void jackel(int level, int x, int y)
{
  entity_t *monster;
  entity_new(&monster, IDENT_NPC, "JACKEL");
  comp_position(monster, x, y);
  comp_renderable(monster, 'J'-64, 200, 200, 200, 255);
  comp_speed(monster, 1.0f);
  comp_move(monster);
  comp_stats(monster, 20 + (5 * level), level, 2 + (3 * (level - 1)));
  comp_ai(monster);
  monster->ai.flees = 1;
  monster->ai.dumb = 1;
  monster->ai.hostile = 1;
  monster->stats.expmod = 1;

  //                ######################|######################|######################|######################|
  setdesc(monster, "A JACKEL. FAST AND     VICIOUS. NO USE TRYING TO OUTRUN THIS");
}

void bat(int level, int x, int y)
{
  entity_t *monster;
  entity_new(&monster, IDENT_NPC, "BAT");
  comp_position(monster, x, y);
  comp_renderable(monster, 'B'-64, 120, 120, 120, 255);
  comp_speed(monster, 1.0f);
  comp_move(monster);
  comp_stats(monster, 10 + (5 * level), level, 1 + (1 * (level - 1)));
  comp_ai(monster);
  monster->ai.flees = 0;
  monster->ai.dumb = 1;
  monster->ai.hostile = 1;
  monster->stats.expmod = 1;

  //                ######################|######################|######################|######################|
  setdesc(monster, "A BAT. ITS PRETTY FAST IN FLIGHT BUT WONT PUT UP MUCH OF A FIGHT");
}

void zombie(int level, int x, int y)
{
  entity_t *monster;
  entity_new(&monster, IDENT_NPC, "ZOMBIE");
  comp_position(monster, x, y);
  comp_renderable(monster, 'Z'-64, 120, 255, 120, 255);
  comp_speed(monster, 0.25f);
  comp_move(monster);
  comp_stats(monster, 80 + (8 * level), level, 8 + (5 * (level - 1)));
  comp_ai(monster);
  comp_inventory(monster);
  monster->ai.flees = 0;
  monster->ai.hostile = 1;
  monster->ai.dumb = 1;
  monster->stats.expmod = 3;

  //                ######################|######################|######################|######################|
  setdesc(monster, "A MINDLESS ZOMBIE.     PROBABLY WONT REMEMBER SEEING YOU FOR LONG");
}

void blob(int level, int x, int y, int angry)
{
  entity_t *monster;
  entity_new(&monster, IDENT_NPC, "CREATURE");
  comp_position(monster, x, y);
  comp_renderable(monster, 'O'-64, 0, 255, 180, 255);
  comp_speed(monster, 1.0f);
  comp_move(monster);
  comp_stats(monster, 30 + (5 * level), level, 2 + (1 * (level - 1)));
  comp_ai(monster);
  comp_inventory(monster);
  monster->ai.flees = 1;
  monster->ai.splitter = 1;
  monster->stats.expmod = 1;

  if (angry) {
    monster->ai.aggro = 1;
    monster->ai.target = player->id;
  }

  //                ######################|######################|######################|######################|
  setdesc(monster, "A BUBBLING GELATINOUS  BLOB. RUMOUR HAS IT    THESE SPLIT INTO TWO   WHEN THREATENED");
}

void wizard(int level, int x, int y)
{
  entity_t *monster;
  entity_new(&monster, IDENT_NPC, "WIZARD");
  comp_position(monster, x, y);
  comp_renderable(monster, 'W'-64, 120, 120, 255, 255);
  comp_speed(monster, 0.5f);
  comp_move(monster);
  comp_stats(monster, 60 + (10 * level), level, 2 + (2 * (level - 1)));
  comp_ai(monster);
  comp_inventory(monster);
  inventory_add(monster, ITEM_WAND_LIGHTNING, 20);
  monster->ai.flees = 1;
  monster->stats.expmod = 5;

  //                ######################|######################|######################|######################|
  setdesc(monster, "A CLOAKED WIZARD       EXCEPTIONALLY STRONG   MAGIC CASTER");
}

/*-----------------------------------------/
/---------------- SYSTEMS -----------------/
/-----------------------------------------*/
void system_renderable(entity_t *e)
{
  if (!e->components.renderable || !e->components.position)
    return;

  if (e->ident == IDENT_CONTAINER) {
    for (int i=0; i<ENTITY_STACK_MAX; i++) {
      entity_t *ent = entity_stack[i];
      if (!ent || !ent->alive || ent->ident != IDENT_CONTAINER || ent->id == e->id )
        continue;

      if (ent->position.to[0] != e->position.to[0] || ent->position.to[1] != e->position.to[1])
        continue;

      for (int j=0; j<8; j++) {
        int tx = e->position.to[0] + around[j][0];
        int ty = e->position.to[1] + around[j][1];
        int tile = level.tiles[(ty * level.w) + tx].tile;
        if (tile != BLOCK_DOOR && get_walkable(tile) && !entity_get(tx, ty)) {
          e->position.to[0] = tx;
          e->position.to[1] = ty;
          break;
        }
      }
    }
  }

  if (e->ident == IDENT_CONTAINER) {
    entity_t *on = entity_get(e->position.to[0], e->position.to[1]);
    int ident = on ? on->ident : IDENT_UNKNOWN;
    if (ident == IDENT_PLAYER || ident == IDENT_NPC)
      return;
  }

  entity_tiles.tiles[to_index(e->position.from[0], e->position.from[1])].tile = 0;

  if (!e->alive) {
    entity_tiles.tiles[to_index(e->position.to[0], e->position.to[1])].tile = 0;
    return;
  }
  
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
    int d = dijkstra_lowest(out, e->move.dmap, e->position.to[0], e->position.to[1], TILES_X, TILES_Y);
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
  entity_t *entity = entity_get_npc(to[0], to[1]);
  u32 ident = (entity != NULL) ? entity->ident : IDENT_UNKNOWN;
  if (entity && e->id == entity->id)
    ident = IDENT_UNKNOWN;
  switch (ident) {
    // do bump attack
    case IDENT_PLAYER:
    case IDENT_NPC: {
      if (e->ident != entity->ident && e->ident != IDENT_NPC)
        action_bump(e, entity);
      action_stop(e);
      action_stop(entity);
      e->energy = 0;
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
    case BLOCK_WATER_DEEP:
    case BLOCK_WALL_V:
    case BLOCK_WALL: {
      action_stop(e);

      if (e->ident == IDENT_PLAYER)
        paused = 1;
      break;
    }

    // open door
    case BLOCK_DOOR: {
      if (e->ident == IDENT_PLAYER || (e->ident == IDENT_NPC && !e->ai.dumb))
        action_open(e, to[0], to[1]);
      action_stop(e);
      break;
    }

    // do move
    default: {
      e->position.to[0] = to[0];
      e->position.to[1] = to[1];
      e->energy = 0;
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
}

void system_inventory(entity_t *e)
{
  if (!e->components.inventory || e->energy < ENERGY_MIN)
    return;

  if (e->inventory.get > -1) {
    entity_t *on = NULL;
    for (int i=0; i<ENTITY_STACK_MAX; i++) {
      entity_t *ent = entity_stack[i];

      if (!ent || !ent->alive || !ent->components.position)
        continue;

      if (ent->position.to[0] == e->position.to[0] && ent->position.to[1] == e->position.to[1] && ent->ident == IDENT_CONTAINER) {
        on = ent;
        break;
      }
    }
    int ident = on ? on->ident : IDENT_UNKNOWN;
    if (ident == IDENT_CONTAINER) {
      int added = inventory_add(e, on->container.item, on->container.uses);
      if (added) {
        char buf[128];
        sprintf(buf, "PICKED UP %s", item_info[on->container.item].name);
        ui_popup(e, buf, 255, 255, 120, 255);
        on->alive = 0;
      } else {
        ui_popup(e, "INVENTORY FULL", 255, 255, 120, 255);
      }
    }
    e->inventory.get = -1;
    e->energy = 0;;
    return;
  }

  if (e->inventory.drop > -1) {
    int index = e->inventory.drop;
    int item = e->inventory.items[index];

    char buf[128];
    sprintf(buf, "%s DROPPED", item_info[item].name);
    ui_popup(e, buf, 255, 255, 120, 255);

    container(item, e->inventory.uses[index], e->position.to[0], e->position.to[1]);

    e->inventory.items[index] = 0;
    e->inventory.drop = -1;
    e->energy = 0;;
    return;
  }

  if (e->inventory.use > -1) {
    int index = e->inventory.use;
    int item = e->inventory.items[index];

    if (item > ITEM_GEAR_START) {
      int slot = item_info[item].slot;
      for (int i=0; i<INVENTORY_MAX; i++) {
        if (i != index && e->inventory.equipt[i] && item_info[e->inventory.items[i]].slot == slot) {
          e->inventory.equipt[i] = 0;
        }
      }
      if (e->ident == IDENT_PLAYER) {
        char buf[128];
        if (e->inventory.equipt[index]) {
          sprintf(buf, "YOU TAKE OFF THE %s", item_info[item].name);
          ui_popup(e, buf, 255, 255, 120, 255);
        } else {
          sprintf(buf, "YOU PUT ON THE %s", item_info[item].name);
          ui_popup(e, buf, 255, 255, 120, 255);
        }
      }
      
      e->inventory.equipt[index] = !e->inventory.equipt[index];
    }
    
    int new = 0;
    if (!item_info[item].identified && e->ident == IDENT_PLAYER) {
      db_set(item);
      char buf[128];
      sprintf(buf, "IDENTIFIED %s", item_info[item].name);
      ui_popup(e, buf, 255, 255, 120, 255);
      new = 1;
    }

    if (item > ITEM_POTION_START && item < ITEM_SCROLL_END) {
      /* -----
      DO ITEM SPECIFIC THING
      -------- */
      switch (item) {
        case ITEM_POTION_HEALING: {
          e->stats.health = MIN((e->stats.health + (e->stats.max_health/2) + (rand() % 20)), e->stats.max_health);
          break;
        }
        case ITEM_SCROLL_MAPPING: {
          magic_mapping = 75;
          break;
        }
      }
      if (!new) {
        if (item == ITEM_KEY) {
          if (level.tiles[(e->position.to[1] * level.w) + e->position.to[0]].tile == BLOCK_STAIRS) {
            ui_popup(e, "YOU UNLOCK THE EXIT AND THE KEY DISSOLVES", 255, 255, 120, 255);
            locked = 0;
            e->inventory.items[index] = 0;
          } else {
            ui_popup(e, "I SEE NOTHING TO USE THE KEY ON HERE", 255, 255, 120, 255);
          }
        } else if (item > ITEM_POTION_START && item < ITEM_POTION_END) {
          char buf[128];
          sprintf(buf, "YOU CONSUME %s", item_info[item].name);
          ui_popup(e, buf, 255, 255, 120, 255);
        } else {
          char buf[128];
          sprintf(buf, "YOU READ %s", item_info[item].name);
          ui_popup(e, buf, 255, 255, 120, 255);
        }
      }

      if (item != ITEM_KEY)
        e->inventory.items[index] = 0;
    }
    
    e->inventory.use = -1;
    e->energy = 0;;
    return;
  }

  if (e->inventory.fire > -1) {
    int item = e->inventory.items[e->inventory.fire];
    entity_t *entity = NULL;
    for (int i=0; i<ENTITY_STACK_MAX; i++) {
      entity_t *ent = entity_stack[i];

      if (!ent || !ent->alive || !ent->components.position)
        continue;

      if (ent->position.to[0] == e->inventory.fire_x && ent->position.to[1] == e->inventory.fire_y && ent->ident != IDENT_CONTAINER) {
        entity = ent;
        break;
      }
    }
    u32 ident = (entity != NULL) ? entity->ident : IDENT_UNKNOWN;
    if (!entity) // || e->id == entity->id
      ident = IDENT_UNKNOWN;
    switch (ident) {
      case IDENT_PLAYER:
      case IDENT_NPC: {
        action_damage(e, entity, item_info[item].damage);
        if (item == ITEM_WAND_LIGHTNING) {
          for (int i=0; i<ENTITY_STACK_MAX; i++) {
            if (e->ident == IDENT_NPC)
              break;

            entity_t *ent = entity_stack[i];
            if (!ent || !ent->alive || ent->ident != IDENT_NPC)
              continue;

            // see if we have los
            int sx = entity->position.to[0], sy = entity->position.to[1];
            int done = 0, distance = 0;
            while (!done) {
              done = line(&sx, &sy, ent->position.to[0], ent->position.to[1]);
              if (distance > 10 || !get_solid(level.tiles[(sy*level.w)+sx].tile)) {
                done = 1;
                break;
              }
            
              if (sx == ent->position.to[0] && sy == ent->position.to[1]) {
                action_damage(player, ent, item_info[item].damage / 2);
                done = 1;
                break;
              }
              distance++;
            }
          }
        }
        break;
      }
      case IDENT_UNKNOWN: {
        break;
      }
    }

    if (e->ident == IDENT_PLAYER && !item_info[item].identified) {
      db_set(item);
      char buf[128];
      sprintf(buf, "IDENTIFIED %s", item_info[item].name);
      ui_popup(e, buf, 255, 255, 120, 255);
    }

    e->inventory.uses[e->inventory.fire]--;
    if (e->inventory.uses[e->inventory.fire] <= 0) {
      e->inventory.items[e->inventory.fire] = 0;
      if (e->ident == IDENT_PLAYER) {
        char buf[128];
        sprintf(buf, "%s DISSOLVES IN YOUR HANDS", item_info[item].name);
        ui_popup(e, buf, 255, 255, 120, 255);
      }
    }

    e->inventory.fire = -1;
    e->energy = 0;
    return;
  }
}

void system_ai(entity_t *e)
{
  if (!e || !e->components.ai || e->energy < ENERGY_MIN)
    return;

  // do equipt
  if (e->components.inventory) {
    for (int i=0; i<INVENTORY_MAX; i++) {
      int item = e->inventory.items[i];
      int equipt = e->inventory.equipt[i];
      if (item > ITEM_GEAR_START && item <ITEM_GEAR_END && !equipt) {
        action_use(e, i);
        return;
      }
    }
  }

  // if hostile, scan for target
  int done = 0, found = 0;
  int distance = 0, x = e->position.to[0], y = e->position.to[1];
  err = 999; err2=999;
  if (e->ai.hostile && !e->ai.aggro) {
    while (!done) {
      done = line(&x, &y, player->position.to[0], player->position.to[1]);
      if (!get_solid(level.tiles[(y*level.w)+x].tile) || distance > 10) {
        break;
      }

      distance++;
    }
    if (x == player->position.to[0] && y == player->position.to[1]) {
      e->ai.target = player->id;
      e->ai.aggro = 1;
      char buf[128];
      sprintf(buf, "THE %s NOTICES YOU", e->name);
      ui_popup(player, buf, 255, 255, 120, 255);
    }
  }

  // do wander
  if (!e->ai.aggro) {
    int dx = (-1 + (rand() % 3)) + e->position.to[0];
    int dy = (-1 + (rand() % 3)) + e->position.to[1];
    int tile = level.tiles[(dy * level.w) + dx].tile;
    if (get_solid(tile) && !(rand() % 2)) {
      e->move.target[0] = dx;
      e->move.target[1] = dy;
    }
    e->move.dmap = NULL;
    return;
  }

  if (e->ai.target < 0)
    return;

  entity_t *target = entity_stack[e->ai.target];
  if (!target)
    return;

  // see if target is visible
  done = 0;
  distance = 0; x = e->position.to[0]; y = e->position.to[1];
  err = 999; err2=999;
  while (!done) {
    done = line(&x, &y, target->position.to[0], target->position.to[1]);
    if (!get_solid(level.tiles[(y*level.w)+x].tile) || distance > 30) {
      e->inventory.fire_x = x;
      e->inventory.fire_x = y;
      break;
    }

    distance++;
  }

  int dist_x = abs(e->position.to[0] - target->position.to[0]);
  int dist_y = abs(e->position.to[1] - target->position.to[1]);

  // do flee
  float hp = ((float)e->stats.health / e->stats.max_health);
  if (hp <= 0.3f || (hp <= 0.5f && e->ai.splitter)) {
    if (e->ai.flees && target->ident == IDENT_PLAYER) {
      e->move.dmap = path_from_player;

      if (dist_x < 2 && dist_y < 2 && !(rand() % 2)) {
        action_bump(e, target);
        e->energy = 0;
      }

      // do split
      if (e->ai.splitter && (x != target->position.to[0] || y != target->position.to[1])) {
        for (int i=0; i<2; i++) {
          blob(e->stats.level, e->position.to[0], e->position.to[1], 1);
        }
        action_damage(e, e, 1000);
      } else if (!e->ai.splitter) {
        if (dist_x > 15 || dist_y > 15) {
          e->stats.health = MIN(e->stats.health + 5, e->stats.max_health);
        }
      }
    }
    return;
  }
  
  int item = ITEM_NONE;
  int itemindex = -1;
  if (x == target->position.to[0] && y == target->position.to[1]) {
    // visible, attempt ranged
    // determine if we have a ranged weapon
    if (e->components.inventory) {
      for (int i=0; i<INVENTORY_MAX; i++) {
        int weapon = e->inventory.items[i];

        if (weapon > ITEM_WAND_START && weapon < ITEM_WAND_END) {
          item = weapon;
          itemindex = i;
          break;
        }
      }
    }

    int range = item_info[item].range;

    // fire projectile
    if (item != ITEM_NONE && distance <= range && distance > 2) {
      action_fire(e, itemindex, target->position.to[0], target->position.to[1]);
      projectile.from[0] = e->position.to[0];
      projectile.from[1] = e->position.to[1];
      projectile.to[0] = target->position.to[0];
      projectile.to[1] = target->position.to[1];
      projectile.tile = 68+8;
      projectile.r = 255;
      projectile.g = 120;
      projectile.b = 255;
      projectile.count = 0;
    } else {
      if (dist_x < 2 && dist_y < 2 && (item == ITEM_NONE || !(rand() % 6))) { // && e->speed.speed < (target->speed.speed - 0.01f)
        // do bump attack
        action_bump(e, target);
        e->energy = 0;
      } else {
        // do chase
        if (target->ident == IDENT_PLAYER) {
          e->move.dmap = path_to_player;
          e->move.target[0] = e->position.to[0];
          e->move.target[1] = e->position.to[1];
          if (item != ITEM_NONE && dist_x <= 2 && dist_y <= 2)
            e->move.dmap = path_from_player;
        }
      }
    }
  } else {
    if (!e->ai.dumb) {
      e->move.dmap = path_to_player;
      e->move.target[0] = e->position.to[0];
      e->move.target[1] = e->position.to[1];
    } else {
      e->ai.target = -1;
      e->ai.aggro = 0;
      char buf[128];
      sprintf(buf, "THE %s FORGETS ABOUT YOU", e->name);
      ui_popup(player, buf, 255, 255, 120, 255);
    }
  }
    
}


/*-----------------------------------------/ 
/---------------- ACTIONS -----------------/
/-----------------------------------------*/
void action_move(entity_t *e, u32 x, u32 y)
{
  if (!e || !e->components.move)
    return;

  e->move.dmap = NULL;
  e->move.target[0] = x;
  e->move.target[1] = y;
}

void action_path(entity_t *e, int *path, u32 w, u32 h)
{
  if (!e || !e->components.move || !path)
    return;

  e->move.dmap = path;
  e->move.w = w;
  e->move.h = h;
}

void action_stop(entity_t *e)
{
  if (!e || !e->components.move || !e->components.position)
    return;

  e->move.target[0] = e->position.to[0];
  e->move.target[1] = e->position.to[1];
  e->move.dmap = NULL;
}

void action_open(entity_t *e, u32 x, u32 y)
{
  // do we have energy to move?
  if (!e || e->energy < ENERGY_MIN)
    return;

  int distance = MAX(abs(e->position.to[0] - x), abs(e->position.to[1] - y));
  if (distance > 1)
    return;

  tile_t *tile = &level.tiles[to_index(x, y)];
  switch (tile->tile) {
    case BLOCK_DOOR: {
      tile->tile = BLOCK_DOOR_OPEN;
      e->energy = 0;;
      break;
    }
    case BLOCK_DOOR_OPEN: {
      tile->tile = BLOCK_DOOR;
      e->energy = 0;;
      break;
    }
  }
}

void action_bump(entity_t *a, entity_t *b)
{
  if (!a || !b || !a->components.stats || !b->components.stats)
    return;

  int weapon = 0;
  if (a->components.inventory) {
    for (int i=0; i<INVENTORY_MAX; i++) {
      if (a->inventory.equipt[i] && a->inventory.items[i] > ITEM_GEAR_WEAPON_START) {
        weapon += item_info[a->inventory.items[i]].damage;
      }
    }
  }

  int damage = a->stats.base_damage + weapon + (a->stats.level * 2);

  action_damage(a, b, damage);
}

void action_damage(entity_t *a, entity_t *b, int damage)
{
  if (!b || !b->components.stats)
    return;

  if (!damage) {
    ui_inspect(b);
    return;
  }

  int defense = 0;
  if (b->components.inventory) {
    for (int i=0; i<INVENTORY_MAX; i++) {
      if (b->inventory.equipt[i] && b->inventory.items[i] > ITEM_GEAR_START) {
        defense += item_info[b->inventory.items[i]].armor;
      }
    }
  }

  defense += (b->stats.level * 3);

  damage = MAX(1, damage - defense);

  b->stats.health -= damage;

  ui_print("x", b->position.to[0], b->position.to[1], 255, 0, 0, 200);

  if (b->components.ai) {
    b->ai.aggro  = 1;
    b->ai.target = a->id;
  }

  // print damage
  char buf[128];
  if (b->ident != IDENT_PLAYER) {
    sprintf(buf, "%s IS HIT FOR %i DAMAGE", b->name, damage);
    ui_popup(b, buf, 255, 120, 120, 255);
  }

  // dead
  if (b->stats.health <= 0) {
    b->alive = 0;
    ui_reset();
    system_renderable(b);
    ui_print("@", b->position.to[0], b->position.to[1], 255, 120, 120, 255);
    sprintf(buf, "%s DIES", b->name);
    ui_popup(b, buf, 255, 120, 120, 255);

    a->stats.exp += b->stats.expmod;
    if (a->stats.exp > (a->stats.level * 5)) {
      a->stats.exp = 0;
      a->stats.level++;
      a->stats.max_health += 5 * a->stats.level;
      a->stats.health = a->stats.max_health;
      a->stats.base_damage += 2;
      sprintf(buf, "LEVEL UP! HEALTH AND DAMAGE INCREASED");
      ui_popup(b, buf, 255, 255, 120, 255);
    }

    // drop items
    if (b->components.inventory) {
      for (int i=0; i<INVENTORY_MAX; i++) {
        if (b->inventory.items[i]) {
          container(b->inventory.items[i], b->inventory.uses[i], b->position.to[0], b->position.to[1]);
        }
      }
    }
  }

  if (b->ident == IDENT_PLAYER) {
    action_stop(b);
  }
}

void action_use(entity_t *e, int item)
{
  if (!e || !e->components.inventory)
    return;

  int i = e->inventory.items[item];
  if (i > ITEM_WAND_START && i < ITEM_WAND_END) {
    ui_state = UI_STATE_AIM;
    return;
  }

  e->inventory.use = item;
}

void action_drop(entity_t *e, int item)
{
  if (!e || !e->components.inventory)
    return;

  e->inventory.drop = item;
}

void action_fire(entity_t *e, int item, int x, int y)
{
  if (!e || !e->components.inventory)
    return;

  e->inventory.fire = item;
  e->inventory.fire_x = x;
  e->inventory.fire_y = y;
}

void action_get(entity_t *e)
{
  if (!e || !e->components.inventory)
    return;

  e->inventory.get = 1;
}