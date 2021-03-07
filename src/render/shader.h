/* shader
  Loads and compiles shaders.

  Requires at minimal a vertex and
  fragment shader, can also compile a
  geometry shader if specified.
*/

#ifndef SHADER_H
#define SHADER_H

#define SHADER_PATH "data/shaders/"

#include <stdio.h>
#include <stdlib.h>

#include "main.h"

typedef struct {
  GLuint ID;
  char path[512];
} shader_t;

/**
 * [uniform cache and return shader uniform locations]
 * @param  shader [shader to be used]
 * @param  str    [uniform string]
 * @return        [uniform location]
 *
 * hashes a string key using djb2
 * www.cse.yorku.ca/~oz/hash.html
 */
GLint uniform(GLuint shader, const char *str);

/**
 * [shader (lazy) loads, attaches and links shaders into a shader program]
 * @param  path   [shader file path]
 * @return        [the shader program GLuint]
 *
 * Move this to a .c file, it shouldn't be
 * in the header.
 */

GLuint shader_load(const char *path);

/**
 * [shader_use bind a shader for use assuming it is not already in use]
 * @param shader [shader to use]
 */
void shader_use(GLuint shader);


#endif // SHADER_H
