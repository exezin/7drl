/* texture
  Handles loading and converting
  a texture file to a GL texture.

  Currently uses textures with 4
  components. (rgba)
*/

#ifndef TEXTURE_H
#define TEXTURE_H

#define TEXTURE_LOC "data/textures/"

#include <stdint.h>
#include <stdio.h>
#include <glad/glad.h>
#include "types.h"

typedef struct {
  GLuint id;
  int width, height;
  char name[32];
  u8 *data;
} texture_t;

/**
 * [texture_load load a texture from file]
 * @param  file [file path string]
 * @return      [texture var]
 */
texture_t* texture_load(const char *file, int get_data);

#endif // TEXTURE_H