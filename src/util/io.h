/* io
  Generic io helper functions for
  saving and loading data.
*/

#ifndef IO_H
#define IO_H

#include "util/debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <physfs.h>

/**
 * [io_read reads a file into a char array]
 * @param  path [file path]
 * @param  mode [access mode]
 * @return      [malloc'd char array, remember to free]
 */
static char* io_read(const char *path, const char *mode, size_t *len)
{
  if (!PHYSFS_exists(path)) {
    P_DBG("Could not load file %s\n", path);
    return NULL;
  }

  // load the file
  PHYSFS_file *file;
  file = PHYSFS_openRead(path);

  // get file length
  size_t size = PHYSFS_fileLength(file);
  if (len)
    *len = size;

  // allocate space for file data
  char *buff = malloc(size+1);

  // read file contents into buffer
  PHYSFS_readBytes(file, buff, size);

  // null-terminate the buffer
  buff[size] = '\0';

  PHYSFS_close(file);

  return buff;
}


/**
 * [io_prefix_str prefix a string with another]
 * @param  dest   [destination string]
 * @param  src    [input string]
 * @param  prefix [prefix string]
 * @return        [input string with prefix]
 */
static inline void io_prefix_str(char *dest, const char *src, const char *prefix)
{
  if (src == NULL) {
    dest[0] = '\0';
    return;
  }

  size_t len = strlen(prefix);
  strcpy(dest, prefix);
  strcpy(&dest[len], src);
}

/**
 * [io_get_extension get the file extension string]
 * @param dest [destination string]
 * @param src  [source string]
 */
static inline void io_get_extension(char *dest, const char *src)
{
  dest[0] = '\0';

  if (src == NULL)
    return;

  char *extension = strrchr(src, '.');
  if (extension != NULL)
    strcpy(dest, extension);
}

/**
 * [io_write writes bytes to a file]
 * @param path   [path to file]
 * @param data   [data to write]
 * @param len    [length of data]
 * @param append [if set to 1, append to end of file]
 */
static inline void io_write(const char *path, const void *data, size_t len, int append)
{
  // attempt to open file for writing
  PHYSFS_File *file = NULL;
  if (append)
    file = PHYSFS_openAppend(path);
  else
    file = PHYSFS_openWrite(path);

  // check that file was opened
  if (!file) {
    P_ERR("Unable to open file %s for writing\n", path);
    return;
  }

  // attempt to write bytes to file
  if (PHYSFS_writeBytes(file, (void*)data, len) < len) {
    P_ERR("Unable to write bytes to file %s\n", path);
    return;
  }

  PHYSFS_close(file);
}

#endif // IO_H