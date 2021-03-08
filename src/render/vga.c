#include "render/vga.h"
#include "render/vga_font.h"
#include "render/shader.h"
#include "render/render.h"
#include "db.h"
#include "math/linmath.h"
#include <stdio.h>

#define VGA_FONT_WIDTH  8
#define VGA_FONT_HEIGHT 16
#define VGA_WIDTH  432
#define VGA_HEIGHT 261

static u32 *vga_data = NULL;
static u32 vga_fg = 0xFFFFFFFF, vga_bg = 0x00000000;
static size_t vga_len = 0;
static GLuint texture, shader, vao, vbo;
static GLfloat vertices[24];
static mat4x4 projection;

void vga_init()
{
  if (vga_data) {
    free(vga_data);
  } else {
    // gen texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0,
      GL_RGBA, VGA_WIDTH, VGA_HEIGHT, 0,
      GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // load shader
    shader = shader_load("vga.glsl");

    // set up vao, vbo etc
    float w = VGA_WIDTH;
    float h = VGA_HEIGHT;
    float u0 = 0.0f, v0 = 0.0f;
    float u1 = 1.0, v1 = 1.0;
    GLfloat v[] = {
      // pos          // uv
      0.0f,   0.0f+h, u0,  v1,
      0.0f,   0.0f,   u0,  v0,
      0.0f+w, 0.0f,   u1,  v0,
      0.0f,   0.0f+h, u0,  v1,
      0.0f+w, 0.0f,   u1,  v0,
      0.0f+w, 0.0f+h, u1,  v1
    };
    memcpy(vertices, v, sizeof(GLfloat)*24);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    u32 stride = sizeof(GLfloat) * 4;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*24, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * 2));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // calculate vga data len
  vga_len = VGA_FONT_WIDTH * VGA_FONT_HEIGHT;
  vga_len *= VGA_WIDTH * VGA_HEIGHT;

  // malloc a large buffer for the pixel data
  vga_data = malloc(sizeof(u32) * vga_len);
  memset(vga_data, 0, sizeof(u32) * vga_len);

  P_DBG("VGA system initialized\n");
}

void vga_print(size_t x, size_t y, const char *str)
{
  x *= VGA_FONT_WIDTH;
  y *= VGA_FONT_HEIGHT;

  u8 *font = vga_font_array;
  for (int i=0; i<strlen(str); i++) {
    size_t c = (str[i]*16);

    if (c > VGA_FONT_DATA_LEN)
      c = 0;

    if (x >= VGA_WIDTH * VGA_FONT_WIDTH) {
      x = 0;
      y += VGA_FONT_HEIGHT;
    }
    if (y >= VGA_HEIGHT * VGA_FONT_HEIGHT)
      y = 0;

    for (int j=0; j<16; j++) {
      size_t p = (VGA_WIDTH * y) + x;
      p += (VGA_WIDTH * j);
      u8 byte = font[c+j];

      for (int k=0; k<8; k++) {
        int color = (byte >> k) & 0x01;
        if (color)
          vga_data[p+(8-k)] = vga_fg;
        else
          vga_data[p+(8-k)] = vga_bg;
      }
    }

    x += VGA_FONT_WIDTH;
  }
}

void vga_render()
{
  mat4x4_ortho(projection, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, -1.0f, 1.0f);
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
    VGA_WIDTH, VGA_HEIGHT,
    GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, vga_data);

  glUseProgram(shader);
  glBindVertexArray(vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(uniform(shader, "u_texture"), 0);
  glUniformMatrix4fv(uniform(shader, "u_projection"), 1, GL_FALSE, projection[0]);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisable(GL_BLEND);

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void vga_clear()
{
  memset(vga_data, 0, sizeof(u32) * vga_len);
}

void vga_setfg(u8 r, u8 g, u8 b, u8 a)
{
  vga_fg = r;
  vga_fg |= ((u32)g) << 8;
  vga_fg |= ((u32)b) << 16;
  vga_fg |= ((u32)a) << 24;
}

void vga_setbg(u8 r, u8 g, u8 b, u8 a)
{
  vga_bg = r;
  vga_bg |= ((u32)g) << 8;
  vga_bg |= ((u32)b) << 16;
  vga_bg |= ((u32)a) << 24;
}

void vga_clean()
{
  if (vga_data) {
    P_DBG("Cleaning up vga\n");
    free(vga_data);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
  }
}