#include "render/shader.h"
#include "util/io.h"
#include <string.h>

#define MAX_SHADERS 512

GLint uniform_map[256][256] = {{0}};
GLint uniform_locations[256][256] = {{0}};

shader_t shader_list[MAX_SHADERS];
size_t shader_count = 0;

GLuint active_shader = 0;

inline GLint uniform(GLuint shader, const char *str)
{
  const char *string = str;
  u32 key = 5381;
  int c;

  // hash * 33 + c
  while ((c = *str++))
    key = ((key << 5) + key) + c;

  // check if location cached already
  int i=0;
  for (i=0; i<256; i++) {
    // end of array
    if (!uniform_map[shader][i])
      break;

    // check cached
    if (uniform_map[shader][i] == key)
      return uniform_locations[shader][i];
  }

  // store and return it
  GLint value = glGetUniformLocation(shader, string);
  uniform_map[shader][i] = key;
  uniform_locations[shader][i] = value;

  return value;
}

GLuint shader_load(const char *path)
{
  // check if shader is already loaded
  for (int i=0; i<MAX_SHADERS; i++)
    if (strcmp(shader_list[i].path, path) == 0)
      return shader_list[i].ID;

  // prefix path with shader dir
  char real_path[256];
  io_prefix_str(real_path, path, SHADER_PATH);

  char *str = io_read(real_path, "r", NULL);
  char *shaders[3] = {NULL, NULL, NULL};
  const char *types[][2] = {
    {"#START VS", "#END VS"},
    {"#START FS", "#END FS"},
    {"#START GS", "#END GS"}
  };

  // extract shaders
  for (int i=0; i<3; i++) {
    char *start = strstr(str, types[i][0]);
    char *end   = strstr(str, types[i][1]);
    if (start && end) {
      size_t len = (end - start)  - 10;
      shaders[i] = malloc(len);
      strncpy(shaders[i], &start[10], len);
      shaders[i][len-1] = '\0';
    }
  }

  // create the shaders
  GLuint vertshader, fragment_shader, geometry_shader;
  vertshader   = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);

  // compile the shaders
  glShaderSource(vertshader, 1, (const GLchar* const*)&shaders[0], NULL);
  glCompileShader(vertshader);

  GLuint shader_program = 0;

  GLint success = 0;
  GLchar compile_log[512];
  glGetShaderiv(vertshader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertshader, 512, NULL, compile_log);
    P_ERR("Failed to compile vertex shader: %s\n", compile_log);
    goto exit;
  }

  glShaderSource(fragment_shader, 1, (const GLchar* const*)&shaders[1], NULL);
  glCompileShader(fragment_shader);

  success = 0;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, compile_log);
    P_ERR("Failed to compile fragment shader: %s\n", compile_log);
    goto exit;
  }

  if (shaders[2] != NULL) {
    glShaderSource(geometry_shader, 1, (const GLchar* const*)&shaders[2], NULL);
    glCompileShader(geometry_shader);

    success = 0;
    glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(geometry_shader, 512, NULL, compile_log);
      P_ERR("Failed to compile geometry shader: %s\n", compile_log);
      goto exit;
    }
  }

  // create shader program
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertshader);
  glAttachShader(shader_program, fragment_shader);
  if (shaders[2] != NULL)
   glAttachShader(shader_program, geometry_shader);
  glLinkProgram(shader_program);

  success = 0;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, compile_log);
    P_ERR("Failed to link shader program: %s\n", compile_log);
    goto exit;
  }

exit:
  glDeleteShader(vertshader);
  glDeleteShader(fragment_shader);
  glDeleteShader(geometry_shader);

  for (int i=0; i<3; i++) {
    if (shaders[i] != NULL)
      free(shaders[i]);
  }

  P_DBG("Shaders (%s) successfully compiled\n", path);

  if (shader_program) {
    shader_list[shader_count].ID = shader_program;
    strcpy(shader_list[shader_count].path, path);
    if (shader_count < MAX_SHADERS)
      shader_count++;
    else
      P_ERR("Max number of shaders reached!\n");
  }

  return shader_program;
}

void shader_use(GLuint shader)
{
  if (active_shader == shader)
    return;

  glUseProgram(shader);
}