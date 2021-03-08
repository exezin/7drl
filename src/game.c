#include <time.h>
#include "game.h"
#include "gen.h"
#include "entity.h"
#include "ui.h"
#include "render/render.h"
#include "render/vga.h"
#include "input/input.h"

// delta time vars
static const double phys_delta_time = 1.0 / 60.0;
static const double slowest_frame = 1.0 / 15.0;
static double delta_time, accumulator = 0.0;
static double last_frame_time = 0.0;
double game_tick = 0.0;

// set to zero when the player wants to take a turn
// set to one when the player has sufficient energy to take a turn
int paused = 0;

double projectile_timer = 0;
projectile_t projectile = {0};

int entity_index = 0;

int use_item = 0;
int tile_on = 0;

void (*direction_action)(entity_t*, u32, u32) = NULL;

// keybinds with associated action
keybind_t keybinds[SDL_NUM_SCANCODES] = {0};

// visible tile maps
tilesheet_packet_t level = {NULL}, entity_tiles = {NULL};

int dungeon_depth = 0;

// player stuff
entity_t *player = NULL, *monster;

// dijkstra maps
int path_to_player[TILES_NUM] = {0}, path_from_player[TILES_NUM] = {0}, path_to_mouse[TILES_NUM] = {0};

// prototypes
void game_action_door();
void game_action_wait();
void game_action_left();
void game_action_right();
void game_action_up();
void game_action_down();
void game_action_upleft();
void game_action_downleft();
void game_action_upright();
void game_action_downright();
void game_action_use_item();
void game_action_drop_item();
void game_action_inventory();
void game_action_fire();
void game_action_use();


/*-----------------------------------------/
/---------------- MISC --------------------/
/-----------------------------------------*/
void projectile_start(int fromx, int fromy, int tox, int toy, int t)
{
  projectile.from[0] = fromx;
  projectile.from[1] = fromy;
  projectile.to[0] = tox;
  projectile.to[1] = toy;
  projectile.tile = t;
  projectile.r = 255;
  projectile.g = 255;
  projectile.b = 255;

  tile_t *tile = &entity_tiles.tiles[to_index(projectile.from[0], projectile.from[1])];
  tile->tile = projectile.tile;
  tile->r    = projectile.r;
  tile->g    = projectile.g;
  tile->b    = projectile.b;
  tile->a    = 255;

  projectile_timer = PROJECTILE_SPEED;
}

int projectile_run()
{
  // update projectile
  projectile_t *p = &projectile;
  if (p->tile) {
    if (projectile_timer > 0.0f)
      return 1;

    tile_t *tile = &entity_tiles.tiles[to_index(p->from[0], p->from[1])];
    tile->tile   = 0;

    line(&(p->from[0]), &(p->from[1]), p->to[0], p->to[1]);

    tile       = &entity_tiles.tiles[to_index(p->from[0], p->from[1])];
    tile->tile = p->tile;
    tile->r    = p->r;
    tile->g    = p->g;
    tile->b    = p->b;
    tile->a    = 255;

    if (p->from[0] == p->to[0] && p->from[1] == p->to[1]) {
      entity_tiles.tiles[to_index(p->from[0], p->from[1])].tile = 0;
      p->tile = 0;
    }

    projectile_timer = PROJECTILE_SPEED;
    return 1;
  }

  return 0;
}

void generate_dungeon(int depth)
{
  // generate the dungeon
  gen(&level);

  // move the player into position
  int x, y;
  int tile = 0;
  while (tile != BLOCK_FLOOR) {
    x = rand() % level.w;
    y = rand() % level.h;
    tile = level.tiles[(y * level.w) + x].tile;
  }
  comp_position(player, x, y);
  player->position.to[1] -= 1;
  action_move(player, player->position.to[0], player->position.to[1]+1);
}


/*-----------------------------------------/
/---------------- CORE STUFF --------------/
/-----------------------------------------*/
ERR game_init()
{
  srand(time(NULL));
  // srand(5);
  
  // initialize the renderer
  if (render_init() == SUCCESS) {
    P_DBG("Renderer initialized\n");
  } else {
    P_ERR("Error initializing renderer\n");
    return FAILURE;
  }

  // for delta time
  last_frame_time = SDL_GetPerformanceCounter();

  // default keybinds
  keybinds[SDL_SCANCODE_A].action = &game_action_left;
  keybinds[SDL_SCANCODE_H].action = &game_action_left;
  keybinds[SDL_SCANCODE_D].action = &game_action_right;
  keybinds[SDL_SCANCODE_L].action = &game_action_right;
  keybinds[SDL_SCANCODE_W].action = &game_action_up;
  keybinds[SDL_SCANCODE_K].action = &game_action_up;
  keybinds[SDL_SCANCODE_S].action = &game_action_down;
  keybinds[SDL_SCANCODE_J].action = &game_action_down;
  keybinds[SDL_SCANCODE_Q].action = &game_action_upleft;
  keybinds[SDL_SCANCODE_Y].action = &game_action_upleft;
  keybinds[SDL_SCANCODE_Z].action = &game_action_downleft;
  keybinds[SDL_SCANCODE_B].action = &game_action_downleft;
  keybinds[SDL_SCANCODE_E].action = &game_action_upright;
  keybinds[SDL_SCANCODE_U].action = &game_action_upright;
  keybinds[SDL_SCANCODE_C].action = &game_action_downright;
  keybinds[SDL_SCANCODE_N].action = &game_action_downright;

  keybinds[SDL_SCANCODE_O].action = &game_action_door;
  keybinds[SDL_SCANCODE_PERIOD].action = &game_action_wait;

  keybinds[SDL_SCANCODE_I].action = &game_action_inventory;
  keybinds[SDL_SCANCODE_F].action = &game_action_fire;
  keybinds[SDL_SCANCODE_U].action = &game_action_use;

  // initialize the item db
  db();

  // tilemap specifically for entities
  entity_tiles.x = 0, entity_tiles.y = 0;
  entity_tiles.zoom = 1;
  entity_tiles.w = (WINDOW_WIDTH / TILE_RWIDTH), entity_tiles.h = (WINDOW_HEIGHT / TILE_RHEIGHT);
  entity_tiles.rx = 0, entity_tiles.rw = entity_tiles.w;
  entity_tiles.ry = 0, entity_tiles.rh = entity_tiles.h;
  entity_tiles.tiles = calloc(1, sizeof(tile_t) * entity_tiles.w * entity_tiles.h);

  // initialize the player entity
  entity_new(&player, IDENT_PLAYER, "PLAYER");
  comp_renderable(player, 38, 255, 255, 255, 255);
  comp_speed(player, 1.0f);
  comp_move(player);
  comp_stats(player, 100, 1, 5);
  comp_inventory(player);
  inventory_add(player, ITEM_POTION_HEALING, 1);
  inventory_add(player, ITEM_SCROLL_MAPPING, 1);
  inventory_add(player, ITEM_WAND_FIREBOLT, 10);
  inventory_add(player, ITEM_GEAR_CHAINHELM, 1);
  inventory_add(player, ITEM_GEAR_CHAINHELM, 1);

  generate_dungeon(dungeon_depth);

  // initialize the player entity
  for (int i=0; i<10; i++) {
    int x, y;
    int tile = 0;
    while (tile != BLOCK_FLOOR) {
      x = rand() % level.w;
      y = rand() % level.h;
      tile = level.tiles[(y * level.w) + x].tile;
    }
    entity_new(&monster, IDENT_NPC, "GOBLIN");
    comp_position(monster, x, y);
    comp_renderable(monster, 'G'-64, 255, 255, 255, 255);
    comp_speed(monster, 0.5f);
    comp_move(monster);
    comp_stats(monster, 50, 1, 5);
    // if (monster)
      action_path(monster, path_to_player, TILES_X, TILES_Y);
  }
 
  return SUCCESS;
}

int game_run()
{
  int running = 1;

  /*-----------------------------------------/
  /---------------- UPDATE ------------------/
  /-----------------------------------------*/
  // calculate delta time
  double current_frame_time = (double)SDL_GetPerformanceCounter();
  delta_time = (double)(current_frame_time - last_frame_time) / (double)SDL_GetPerformanceFrequency();
  last_frame_time = current_frame_time;

  // prevent spiral of death
  if (delta_time > slowest_frame)
    delta_time = slowest_frame;

  game_tick += delta_time;
  projectile_timer -= delta_time;

  // update at a constant rate to keep physics in check
  accumulator += delta_time;
  while (accumulator >= phys_delta_time) {
    // handle events etc
    if (!render_update())
      running = 0;

    // do game update
    game_update(phys_delta_time, delta_time);

    if (!player->alive)
      paused = 0;

    // handle entities
    float energy = player->energy;
    for (int i=entity_index; i<ENTITY_STACK_MAX; i++) {
      if (ui_rendering)
        break;

      if (projectile_run()) {
        break;
      }

      // update entity
      entity_t *e = entity_stack[i];
      if (!e)
        continue;

      // are we already paused?
      if (paused && !entity_index)
        break;

      system_energy(e);
      system_stats(e);
      system_move(e);
      system_inventory(e);
      system_renderable(e);

      // did this entity spawn a projectile?
      if (projectile.tile) {
        entity_index = i+1;
        paused = 0;
        break;
      }

      entity_index = 0;

      if (!e->alive && e->ident != IDENT_PLAYER)
        entity_remove(e->id);

      // an action might have not performed
      // thus causing a re-pause
      if (paused)
        break;
    }

    player_path(player);

    if (player->energy >= ENERGY_MIN && !player->move.dmap)
      paused = 1;

    // dec accumulator
    accumulator -= phys_delta_time;
  }
  /*----------------------------------------*/


  /*-----------------------------------------/
  /---------------- RENDER ------------------/
  /-----------------------------------------*/
  // dbg render dt stuff
  // char buff[512];
  // sprintf(buff, "fps: %.2f, dt: %.4f, step %.2f", 1.0 / delta_time, delta_time, 1.0 / phys_delta_time);
  // vga_print(1, 1, buff);

  // do render pass
  render_render();

  // exit
  if (!running) {
    render_clean();
    vga_clean();
  }
  /*----------------------------------------*/

  return running;
}

void game_keypressed(SDL_Scancode key)
{
  if (ui_state != UI_STATE_ITEM)
    use_item = key_to_num(key);

  if (key == SDL_SCANCODE_ESCAPE) {
    ui_state = UI_STATE_NONE;
    ui_reset();
  }

  switch (ui_state) {
    case UI_STATE_INVENTORY: {
      ui_item(player, use_item);
      break;
    }
    case UI_STATE_FIRE: {
      if (player->inventory.items[use_item] > ITEM_WAND_START &&
          player->inventory.items[use_item] < ITEM_WAND_END) {
        game_action_use_item();
      }
      ui_reset();
      break;
    }
    case UI_STATE_USE: {
      if (player->inventory.items[use_item] < ITEM_SCROLL_END) {
        game_action_use_item();
      }
      ui_reset();
      break;
    }
    case UI_STATE_ITEM: {
      if (key == SDL_SCANCODE_A)
        game_action_use_item();
      if (key == SDL_SCANCODE_B)
        game_action_drop_item();
      ui_reset();
      break;
    }
    default: {
      if (keybinds[key].action) {
        ui_reset();
        keybinds[key].action();
      }
      break;
    }
  }
}

void game_mousepressed(int button)
{
  // translate mouse to tile coords
  int mx = mouse_x, my = mouse_y;
  render_translate_mouse(&mx, &my);
  
  ui_reset();

  // handle contextual clicks
  if (button == 3 && !ui_rendering) {
    int tile = level.tiles[to_index(mx, my)].tile;
    int entity_tile = entity_tiles.tiles[to_index(mx, my)].tile;
    entity_t *entity = NULL;
    if (entity_tile)
      entity = entity_get(mx, my);
    u32 ident = (entity != NULL) ? entity->ident : IDENT_UNKNOWN;
    if (entity && player->id == entity->id)
      ident = IDENT_UNKNOWN;
    switch (ident) {

    }

    switch (tile) {
      case BLOCK_DOOR_OPEN:
      case BLOCK_DOOR: {
        action_open(player, mx, my);
        break;
      }
    }
  } else {
    int dist = MAX(abs(player->position.to[0] - mx), abs(player->position.to[1] - my));

    // calculate dmap to mouse
    if (dist < 2) {
      action_move(player, mx, my);
    } else {
      dijkstra(path_to_mouse, mx, my, TILES_X, TILES_Y);
      action_path(player, path_to_mouse, TILES_X, TILES_Y);
    }
  }
  paused = 0;
}

void game_mousewheel(int dx, int dy)
{

}

void game_mousemotion(int dx, int dy)
{

}

/*-----------------------------------------/
/---------------- LOGIC -------------------/
/-----------------------------------------*/
void game_update(double step, double dt)
{
  tile_on = level.tiles[(player->position.to[1] * TILES_X) + player->position.to[0]].tile;
}

void game_render()
{
  for (int i=0; i<TILES_NUM; i++) {
    tile_t *tile = &level.tiles[i];
    if (tile->a <= 50.0f)
      continue;

    if (fov_alpha[i] <= 50.0f)
      continue;

    if ((tile->tile == BLOCK_WATER || tile->tile == BLOCK_WATER_DEEP || tile->tile == BLOCK_FLOOR+1) && !(rand() % 200))
      tile->a = fov_alpha[i] - (rand() % (fov_alpha[i]/4));
  }

  ui_character(player);

  render_tilemap(&level);
  render_tilemap(&entity_tiles);
}

/*-----------------------------------------/
/---------------- ACTIONS -----------------/
/-----------------------------------------*/
void game_action_door()
{
  direction_action = action_open;

  ui_print_entity(player, "OPEN IN WHICH DIRECTION?", 5, 255, 255, 255, 255);
  ui_rendering = 0;
}

void game_action_wait()
{
  paused = 0;
}

void game_action_left()
{
  if (direction_action)
    direction_action(player, player->position.from[0]-1, player->position.from[1]);
  else
    action_move(player, player->position.from[0]-1, player->position.from[1]);

  direction_action = NULL;
  paused = 0;
}

void game_action_right()
{
  if (direction_action)
    direction_action(player, player->position.from[0]+1, player->position.from[1]);
  else
    action_move(player, player->position.from[0]+1, player->position.from[1]);
  
  direction_action = NULL;
  paused = 0;
}

void game_action_up()
{
  if (direction_action)
    direction_action(player, player->position.from[0], player->position.from[1]-1);
  else
    action_move(player, player->position.from[0], player->position.from[1]-1);

  direction_action = NULL;
  paused = 0;
}

void game_action_down()
{
  if (direction_action)
    direction_action(player, player->position.from[0], player->position.from[1]+1);
  else
    action_move(player, player->position.from[0], player->position.from[1]+1);

  direction_action = NULL;
  paused = 0;
}

void game_action_upleft()
{
  if (direction_action)
    direction_action(player, player->position.from[0]-1, player->position.from[1]-1);
  else
    action_move(player, player->position.from[0]-1, player->position.from[1]-1);

  direction_action = NULL;
  paused = 0;
}

void game_action_downleft()
{
  if (direction_action)
    direction_action(player, player->position.from[0]-1, player->position.from[1]+1);
  else
    action_move(player, player->position.from[0]-1, player->position.from[1]+1);

  direction_action = NULL;
  paused = 0;
}

void game_action_upright()
{
  if (direction_action)
    direction_action(player, player->position.from[0]+1, player->position.from[1]-1);
  else
    action_move(player, player->position.from[0]+1, player->position.from[1]-1);

  direction_action = NULL;
  paused = 0;
}

void game_action_downright()
{
  if (direction_action)
    direction_action(player, player->position.from[0]+1, player->position.from[1]+1);
  else
    action_move(player, player->position.from[0]+1, player->position.from[1]+1);

  direction_action = NULL;
  paused = 0;
}

void game_action_use_item()
{
  action_use(player, use_item);

  paused = 0;
}

void game_action_drop_item()
{
  action_drop(player, use_item);

  paused = 0;
}

void game_action_inventory()
{
  ui_inventory(player);
}

void game_action_fire()
{
  ui_fire(player);
}

void game_action_use()
{
  ui_use(player);
}