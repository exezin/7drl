#include "render/render.h"
#include "render/texture.h"
#include "render/shader.h"
#include "render/vga.h"
#include "input/input.h"
#include "math/linmath.h"
#include "game.h"
#include "ui.h"
#include "db.h"

/*---------------- VARS ------------------*/
static mat4x4 projection;
static GLuint sprite_shader, quad_shader, bloom_shader;
static GLuint active_shader;

typedef struct {
  GLuint framebuffer, textures[16];
  u32 width, height, count;
} canvas_t;

static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;

// canvases
static canvas_t screen_canvas, hblur_canvas, vblur_canvas;

extern tilesheet_packet_t ui_tiles;

typedef struct {
  GLuint vao, vbo, shader;
  u32 tile_count;
  float    *vertices;
  u32 vertex_count;
  u32 width,  height;
  u32 rwidth, rheight;
  mat4x4   transform;
  texture_t *texture;
  float *tile_uv;
  int initialized;
} render_tilemap_t;

render_tilemap_t tilemap = { 0 };
/*----------------------------------------*/


/*---------------- HELPERS ---------------*/
void use_shader(GLuint shdr)
{
  active_shader = shdr;
  glUseProgram(active_shader);
}

void render_translate_mouse(int *mx, int *my)
{
  // TODO: clean this up, store some global mouse translation state
  float x_scale = (float)window_width() / (float)WINDOW_WIDTH;
  float y_scale = (float)window_height() / (float)WINDOW_HEIGHT;
  float scale = MIN(x_scale, y_scale);
  float res_x = (float)WINDOW_WIDTH * scale, res_y = (float)WINDOW_HEIGHT * scale;
  float mxoff = ((float)window_width()-(float)WINDOW_WIDTH*scale)/2.0f;
  float myoff = ((float)window_height()-(float)WINDOW_HEIGHT*scale)/2.0f;

  float fmx = (float)(*mx) - (10.0f + mxoff);
  float fmy = (float)(*my) - (10.0f + myoff);
  float fww = (float)res_x - 20.0f;
  float fwh = (float)res_y - 20.0f;
  float x = (fmx / fww) * 2.0 - 1.0;
  float y = (fmy / fwh) * 2.0 - 1.0;

  float cx = 2.25, cy = 2.0;

  float xoff = fabs(y) / 2.25f;
  float yoff = fabs(x) / 2.0f;
  x = x + x * xoff * xoff;
  x = x * 0.5 + 0.5;
  y = y + y * yoff * yoff;
  y = y * 0.5 + 0.5;

  x = CLAMP(x, 0.0f, 0.98f);
  y = CLAMP(y, 0.0f, 0.98f);

  *mx = (int)(x*WINDOW_WIDTH) / TILE_RWIDTH;
  *my = (int)(y*WINDOW_HEIGHT) / TILE_RHEIGHT;
}

// prototypes
void render_tilemap(tilesheet_packet_t *packet);

canvas_t render_new_canvas(u32 w, u32 h, GLenum icf, GLenum ecf, GLenum ctype, GLenum depth, u8 count);
void render_canvas(canvas_t canvas, GLuint texture);
void render_destroy_canvas(canvas_t canvas);
void render_resize(u32 w, u32 h);
/*----------------------------------------*/


/*---------------- MAIN RENDER PASS ------*/
ERR render_init()
{
  // initialize SDL2
  if (!SDL_Init(SDL_INIT_EVERYTHING)) {
    P_DBG("SDL2 initialized\n");
  } else {
    P_ERR("Unable to init SDL2: %s\n", SDL_GetError());
    return FAILURE;
  }

  // set gl attributes
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);

  // get conf vars
  size_t width  = (size_t)ini_get_float(conf, "graphics", "window_width");
  size_t height = (size_t)ini_get_float(conf, "graphics", "window_height");

  // set conf vars if unset
  if (!width) {
    width = WINDOW_WIDTH;
    ini_set_float(conf, "graphics", "window_width", (float)width);
  }
  if (!height) {
    height = WINDOW_HEIGHT;
    ini_set_float(conf, "graphics", "window_height", (float)height);
  }

  // create the window
  window = SDL_CreateWindow(WINDOW_TITLE,
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            width,
                            height,
                            WINDOW_FLAGS);
  if (window != NULL) {
    P_DBG("Window created %lux%lu\n", width, height);
  } else {
    P_ERR("Unable to open window\n");
    return FAILURE;
  }

  // get the gl context
  context = SDL_GL_CreateContext(window);
  if (context != NULL) {
    P_DBG("Got OpenGL context\n");
  } else {
    P_ERR("Failed creating OpenGL context\n");
    return FAILURE;
  }

  // get the gl proc address
  if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    P_DBG("Got OpenGL proc address\n");
  } else {
    P_ERR("Failed getting OpenGL proc address\n");
    return FAILURE;
  }

  // set rendering state
  int vsync = (int)ini_get_float(conf, "graphics", "vsync");
  SDL_GL_SetSwapInterval(vsync);
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glCullFace(GL_BACK);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);
  glDisable(GL_FRAMEBUFFER_SRGB);

  // grab mouse
  SDL_SetRelativeMouseMode(SDL_FALSE);
  SDL_CaptureMouse(SDL_TRUE);
  SDL_SetWindowGrab(window, SDL_FALSE);
  SDL_GL_SetSwapInterval(1);

  // init vga font rendering system
  // primarily for debugging
  vga_init();
  vga_setfg(255, 255, 0, 255);
  vga_setbg(0, 0, 0, 255);

  // setup an ortho projection
  mat4x4_ortho(projection, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0, -1.0f, 1.0f);

  // screen framebuffer
  screen_canvas = render_new_canvas(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_RGBA, GL_FLOAT, GL_FALSE, 1);
  hblur_canvas  = render_new_canvas(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_RGBA, GL_FLOAT, GL_FALSE, 1);
  vblur_canvas  = render_new_canvas(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_RGBA, GL_FLOAT, GL_FALSE, 1);

  // compile shaders
  sprite_shader = shader_load("sprite.glsl");
  quad_shader = shader_load("quad.glsl");
  bloom_shader = shader_load("bloom.glsl");

  ui_init();

  return SUCCESS;
}

void render_render()
{
  glClearColor(0.1, 0.1, 0.15, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // clear framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, screen_canvas.framebuffer);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  game_render();
  ui_render();
  
  // render debug vga font last
  vga_render();
  vga_clear();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  use_shader(bloom_shader);
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  canvas_t *buffers[] = {&vblur_canvas, &hblur_canvas};
  int buffer = 0;
  for (int i=0; i<20; i++) {
    glBindFramebuffer(GL_FRAMEBUFFER, buffers[buffer]->framebuffer);
    glUniform1i(uniform(active_shader, "u_hor"), buffer);

    if (!i)
      render_canvas(screen_canvas, 0);
    else
      render_canvas(*buffers[!buffer], 0);

    buffer = !buffer;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // handle upscaling
  float x_scale = (float)window_width() / (float)WINDOW_WIDTH;
  float y_scale = (float)window_height() / (float)WINDOW_HEIGHT;
  float scale = MIN(x_scale, y_scale);
  float res_x = (float)WINDOW_WIDTH * scale, res_y = (float)WINDOW_HEIGHT * scale;
  float x = ((float)window_width()-(float)WINDOW_WIDTH*scale)/2.0f;
  float y = ((float)window_height()-(float)WINDOW_HEIGHT*scale)/2.0f;
  glViewport(x + 10, y + 10, res_x - 20, res_y - 20);

  use_shader(quad_shader);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, buffers[buffer]->textures[0]);
  glUniform1i(uniform(active_shader, "u_blur"), 1);
  glUniform1f(uniform(active_shader, "u_time"), (float)game_tick);

  render_canvas(screen_canvas, 0);

  SDL_GL_SwapWindow(window);
}

int render_update()
{
  // handle SDL events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
      case SDL_WINDOWEVENT_CLOSE: {
        return 0;
        break;
      }

      // input events
      case SDL_KEYDOWN:
      case SDL_KEYUP:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEWHEEL:
      case SDL_TEXTEDITING:
      case SDL_TEXTINPUT:
      case SDL_MOUSEMOTION:
      case SDL_KEYMAPCHANGED: {
        input_event(&event);
        break;
      }
    }

    // handle window events
    if (event.type == SDL_WINDOWEVENT) {
      switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
          ini_set_float(conf, "graphics", "window_width", (float)event.window.data1);
          ini_set_float(conf, "graphics", "window_height", (float)event.window.data2);
          render_resize(event.window.data1, event.window.data2);
          break;
        }
      }
      break;
    }
  }

  input_update();

  return 1;
}

void render_resize(u32 w, u32 h)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, screen_canvas.framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, hblur_canvas.framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, vblur_canvas.framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);
}

void render_clean()
{
  P_DBG("Cleaning up renderer\n");
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
/*----------------------------------------*/


/*---------------- TILEMAP ---------------*/
void render_tilemap(tilesheet_packet_t *packet)
{
  /*-----------------------------------------/
  /----------------- INIT TILEMAP RENDERER --/
  /-----------------------------------------*/
  if (!tilemap.initialized) {
    glGenVertexArrays(1, &tilemap.vao);
    glGenBuffers(1, &tilemap.vbo);

    u32 stride = sizeof(GLfloat) * 8;

    glBindVertexArray(tilemap.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tilemap.vbo);

    // position (2f)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);

    // uv (2f)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * 2));

    // color (4f)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * 4));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    tilemap.texture = texture_load("font.png", 0);

    // pre-generate tile uvs
    tilemap.tile_count = (tilemap.texture->width / TILE_U) * (tilemap.texture->height / TILE_V);
    tilemap.tile_uv    = malloc(sizeof(float) * (12 * tilemap.tile_count));

    float x = 1.0f, y = 1.0f;
    for (int i=0; i<tilemap.tile_count; i++) {
      float x0 = x / (float)tilemap.texture->width;
      float y0 = y / (float)tilemap.texture->height;
      float x1 = x0 + (TILE_WIDTH / (float)tilemap.texture->width);
      float y1 = y0 + (TILE_HEIGHT / (float)tilemap.texture->height);
      float uvs[] = {
        x0, y1,
        x0, y0,
        x1, y0,
        x0, y1,
        x1, y0,
        x1, y1
      };

      memcpy(&tilemap.tile_uv[i*12], uvs, sizeof(float)*12);

      x += TILE_U;
      if (x > tilemap.texture->width) {
        x = 1.0f;
        y += TILE_V;
      }
    }

    tilemap.rwidth    = packet->rw * (TILE_RWIDTH * packet->zoom);
    tilemap.rheight   = packet->rh * (TILE_RHEIGHT * packet->zoom);

    tilemap.vertex_count = 48 * packet->rw * packet->rh;
    u32 bytes       = sizeof(float) * tilemap.vertex_count;
    tilemap.vertices     = malloc(bytes);

    glBindBuffer(GL_ARRAY_BUFFER, tilemap.vbo);
    glBufferData(GL_ARRAY_BUFFER, bytes, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    tilemap.initialized = 1;
    P_DBG("Tilemap renderer initialized\n");
  }
  /*----------------------------------------*/


  /*-----------------------------------------/
  /----------------- RENDER TILEMAP ---------/
  /-----------------------------------------*/
  // tile render size
  float w = TILE_RWIDTH * packet->zoom;
  float h = TILE_RHEIGHT * packet->zoom;

  // number of tiles we are going to render
  size_t tcount = packet->rw * packet->rh;

  int x = packet->x;
  int y = packet->y;
  float rx = 0.0f, ry = 0.0f;
  for (int i=0; i<tcount; i++) {
    size_t index = i*48;

    if (x < 0 || y < 0)
      goto next;

    // get tile index
    size_t tindex = (y * packet->w) + x;
    size_t tile = 0;
    vec4 rgb = {0.0f};
    if (tindex >= 0 && tindex <= packet->w * packet->h) {
      tile = packet->tiles[tindex].tile;
      rgb[0] = (float)packet->tiles[tindex].r / 255.0f;
      rgb[1] = (float)packet->tiles[tindex].g / 255.0f;
      rgb[2] = (float)packet->tiles[tindex].b / 255.0f;
      rgb[3] = (float)packet->tiles[tindex].a / 255.0f;
    }
    tile *= 12;
    
    if (!tile)
      rgb[3] = 0.0f;

    GLfloat vertices[] = {
      // pos      // uv                                              // color
      rx  , ry+h, tilemap.tile_uv[tile+0],  tilemap.tile_uv[tile+1], rgb[0], rgb[1], rgb[2], rgb[3],
      rx  , ry  , tilemap.tile_uv[tile+2],  tilemap.tile_uv[tile+3], rgb[0], rgb[1], rgb[2], rgb[3],
      rx+w, ry  , tilemap.tile_uv[tile+4],  tilemap.tile_uv[tile+5], rgb[0], rgb[1], rgb[2], rgb[3], 
      rx  , ry+h, tilemap.tile_uv[tile+6],  tilemap.tile_uv[tile+7], rgb[0], rgb[1], rgb[2], rgb[3],
      rx+w, ry  , tilemap.tile_uv[tile+8],  tilemap.tile_uv[tile+9], rgb[0], rgb[1], rgb[2], rgb[3],
      rx+w, ry+h, tilemap.tile_uv[tile+10], tilemap.tile_uv[tile+11], rgb[0], rgb[1], rgb[2], rgb[3]
    };

    memcpy(&tilemap.vertices[index], vertices, sizeof(float)*48);

    next:
    x  += 1;
    rx += w;
    if (rx >= packet->rw * w || x >= packet->w) {
      x  = packet->x;
      y  += 1;
      rx = 0.0f;
      ry += h;
    }
    if (ry >= packet->rh * h || y >= packet->h) {
      break;
    }
  }

  mat4x4_identity(tilemap.transform);
  mat4x4_translate_in_place(tilemap.transform, packet->rx * w, packet->ry * h, 0.0f);

  // bind tile texture
  use_shader(sprite_shader);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tilemap.texture->id);

  // send uniforms
  glUniform2f(uniform(active_shader, "u_uv"), 0.0f, 0.0f);
  glUniform4f(uniform(active_shader, "u_color"), 1.0f, 1.0f, 1.0f, 1.0f);
  glUniform1i(uniform(active_shader, "u_texture"), 0);
  glUniformMatrix4fv(uniform(active_shader, "u_projection"), 1, GL_FALSE, projection[0]);
  glUniformMatrix4fv(uniform(active_shader, "u_model"), 1, GL_FALSE, tilemap.transform[0]);

  // update vbo
  glBindBuffer(GL_ARRAY_BUFFER, tilemap.vbo);
  GLvoid *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*tilemap.vertex_count, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
  memcpy(ptr, tilemap.vertices, sizeof(GLfloat)*tilemap.vertex_count);
  glUnmapBuffer(GL_ARRAY_BUFFER);

  // draw tris
  glBindVertexArray(tilemap.vao);
  glDrawArrays(GL_TRIANGLES, 0, tilemap.vertex_count/4);
  /*----------------------------------------*/
}
/*----------------------------------------*/


/*================ CANVAS ================*/
canvas_t render_new_canvas(u32 w, u32 h, GLenum icf, GLenum ecf, GLenum ctype, GLenum depth, u8 count)
{
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  GLuint textures[count];
  glGenTextures(count, textures);
  for (int i=0; i<count; i++) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);

    glTexImage2D(GL_TEXTURE_2D, 0, icf, w, h, 0, ecf, ctype, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, textures[i], 0);
  }

  GLenum drawbuffers[count];
  for (int i=0; i<count; i++)
   drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
  glDrawBuffers(count, drawbuffers);
  glReadBuffer(GL_NONE);

  if (depth == GL_TRUE) {
    GLuint depth_texture;
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
  }

  canvas_t canvas = {0};
  canvas.framebuffer = framebuffer;
  canvas.width       = w;
  canvas.height      = h;
  canvas.count       = count;
  for (int i=0; i<canvas.count; i++)
    canvas.textures[i] = textures[i];

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    P_ERR("Failed creating framebuffer (%i:%i)\n", w, h);
    canvas.width = 0, canvas.height = 0;
  } else {
    P_DBG("Done creating canvas (%i:%i)\n", w, h);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return canvas;
}

GLuint canvas_vao, canvas_vbo, canvas_init = 0;
void render_canvas(canvas_t canvas, GLuint texture)
{
  if (!canvas_init) {
    glGenVertexArrays(1, &canvas_vao);
    glBindVertexArray(canvas_vao);

    static const GLfloat verts[] = {
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
       1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
       1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenBuffers(1, &canvas_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, canvas_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(float)));

    canvas_init = 1;
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, canvas.textures[texture]);
  glUniform1i(uniform(active_shader, "u_texture"), 0);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glBindVertexArray(canvas_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_destroy_canvas(canvas_t canvas)
{
  P_DBG("Cleaning up a framebuffer\n");

  for (int i=0; i<canvas.count; i++)
    glDeleteTextures(1, &canvas.textures[i]);

  glDeleteFramebuffers(1, &canvas.framebuffer);

  P_DBG("Done destroying canvas\n");
}
/*======================================*/