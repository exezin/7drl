#ifndef DEBUG_H
#define DEBUG_H

// toggle to hide debug output
#define DEBUG_PRINTING

#include <stdio.h>

// debug printing
#ifdef DEBUG_PRINTING
#define P_DBG(f_, ...) printf(("[DBG] " f_), ##__VA_ARGS__)
#else
#define P_DBG(...)
#endif

// error printing
// perhaps replace with log-writes?
#define P_ERR(f_, ...) printf(("[ERR] " f_), ##__VA_ARGS__)

#define PHYSFS_ERR PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())

// error codes
typedef enum {
  SUCCESS = 0,
  ERR_SUCCESS = 0,
  FAILURE,
  ERR_FAILURE,
} ERR;

#endif // DEBUG_H