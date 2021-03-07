/* vga
  Simple vga style font rendering specifically
  for debug information, not for production use.
*/

#ifndef VGA_H
#define VGA_H

#include "main.h"

#include <inttypes.h>
#include <stdlib.h>

/**
 * [vga_init init vga systems]
 */
void vga_init();

/**
 * [vga_print blit some text to the vga]
 * @param x   [x index]
 * @param y   [y index]
 * @param str [ascii string]
 */
void vga_print(size_t x, size_t y, const char *str);

/**
 * [vga_render render vga to the screen]
 */
void vga_render();

/**
 * [vga_clear clear the vga pixel data]
 */
void vga_clear();

/**
 * [vga_setfg set the foreground color]
 * @param r [red]
 * @param g [green]
 * @param b [blue]
 * @param a [alpha]
 */
void vga_setfg(u8 r, u8 g, u8 b, u8 a);

/**
 * [vga_setbg set the background color]
 * @param r [red]
 * @param g [green]
 * @param b [blue]
 * @param a [alpha]
 */
void vga_setbg(u8 r, u8 g, u8 b, u8 a);

/**
 * [vga_clean clean up vga systems]
 */
void vga_clean();

#endif // VGA_H