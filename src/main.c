#define SDL_MAIN_HANDLED 1

#include "main.h"
#include "game.h"

ini_t *conf;

int main(int argc, char **argv)
{
  /*-----------------------------------------/
  /---------------- INIT -------------------*/
  // init physfs filesystem
  if (PHYSFS_init(argv[0]))
    P_DBG("PhysFS initialized\n");
  else
    P_ERR("PhysFS was unable to initialize: %s\n", PHYSFS_ERR);

  // set the safe writing dir
  // most often these directories will be..
  // linux: ~/.local/share/appname
  // windows: AppData\\Roaming\\appname\\appname
  const char *write_path = PHYSFS_getPrefDir(APP_NAME, APP_NAME);
  if (write_path != NULL && PHYSFS_setWriteDir(write_path))
    P_DBG("PhysFS write directory set: %s\n", write_path);
  else
    P_ERR("PhysFS cannot set write directory: %s\n", PHYSFS_ERR);

  // append write path to search paths
  if (PHYSFS_mount(write_path, NULL, 1))
    P_DBG("PhysFS mounted dir %s\n", write_path);
  else
    P_ERR("PhysFS cannot mount dir %s: %s\n", write_path, PHYSFS_ERR);

  // append data path to search paths
  if (PHYSFS_mount(DATA_ZIP_PATH, NULL, 1))
    P_DBG("PhysFS mounted dir %s\n", DATA_ZIP_PATH);
  else
    P_ERR("PhysFS cannot mount dir %s: %s\n", DATA_ZIP_PATH, PHYSFS_ERR);

  // load the config file
  conf = malloc(sizeof(ini_t));
  conf->length = 0;
  if (ini_load(conf, "data/conf.ini") == SUCCESS)
    P_DBG("Loaded base config file\n");
  else
    P_ERR("Failed loading base config file\n");
  /*----------------------------------------*/


  /*-----------------------------------------/
  /---------------- LOOP -------------------*/
  if (game_init() == SUCCESS) {
    P_DBG("Game initialized\n");
  } else {
    P_ERR("Game was unable to initialize\n");
    return FAILURE;
  }

  while (game_run()) {
    // running ...
  }
  /*----------------------------------------*/


  /*-----------------------------------------/
  /---------------- EXIT -------------------*/
  free(conf);

  P_DBG("Clean exit\n");
  return SUCCESS;
  /*----------------------------------------*/
}