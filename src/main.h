#ifndef MAIN_H
#define MAIN_H

#include "util/ini.h"
#include "util/debug.h"

#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <physfs.h>
#include "types.h"

#define APP_NAME "exezinsrl"
#define DATA_ZIP_PATH "data.dat"
#define DATA_PATH "data"

extern ini_t *conf;

#endif // MAIN_H