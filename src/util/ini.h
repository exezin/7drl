/* ini
  A simple ini style config loader, strips all
  white-space from the input file upon loading.

  Supports two variable types, strings and floats,
  make sure you cast values to your required type.
  key=value pairs *must* be in a [section].

  File extension doesn't matter, as long as its a
  text file and not a binary.

  Example conf.ini:

  # [graphics]
  # window_width = 1280
  # quality = high
  #
  # [input]
  # bind_left = a
*/
#ifndef INI_H
#define INI_H

#include "util/debug.h"

#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
  ini_type_undefined,
  ini_type_string,
  ini_type_float,
} ini_type_e;

typedef struct {
  char key[256];
  ini_type_e type;
  union {
    char s[256];
    float f;
  };
} ini_var_t;

typedef struct {
  ini_var_t vars[256];
  char name[256];
  int length;
} ini_section_t;

typedef struct {
  ini_section_t sections[256];
  int length, success;
} ini_t;

/**
 * [ini_load load or update from an ini file]
 * @param  ini  [ini instance to use]
 * @param  path [file path]
 * @return      [non-zero upon success]
 */
ERR ini_load(ini_t *ini, const char *path);

/**
 * [ini_save saves the ini structure to a file]
 * @param ini  [ini to save]
 * @param path [file path]
 */
ERR ini_save(ini_t *ini, const char *path);

/**
 * [ini_get_var get a key-value variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [variable struct]
 */
ini_var_t *ini_get_var(ini_t *ini, const char *sec, const char *key);

/**
 * [ini_get_string get a string variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [string pointer]
 */
char *ini_get_string(ini_t *ini, const char *sec, const char *key);

/**
 * [ini_get_float get a float variable]
 * @param  ini [ini instance to use]
 * @param  sec [variable section]
 * @param  key [variable key]
 * @return     [float value]
 */
float ini_get_float(ini_t *ini, const char *sec, const char *key);

/**
 * [ini_set_string set a string variable]
 * @param ini   [ini instance to use]
 * @param sec   [variable section]
 * @param key   [variable key]
 * @param value [string value]
 */
void ini_set_string(ini_t *ini, const char *sec, const char *key, const char *value);

/**
 * [ini_set_float set a float variable]
 * @param ini   [ini instance to use]
 * @param sec   [variable section]
 * @param key   [variable key]
 * @param value [float value]
 */
void ini_set_float(ini_t *ini, const char *sec, const char *key, const float value);

#endif // INI_H