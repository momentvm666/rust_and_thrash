#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

#define SCREEN_W 320
#define SCREEN_H 240

typedef enum { MODE_13H, MODE_X_240 } video_mode_t;

// Exposed for performance-critical code in road.c
extern unsigned char *vga_mem;
extern int active_page_offset;

bool init_graphics(video_mode_t mode);
void shutdown_graphics();

void gfx_draw_span(int y, int x1, int x2, unsigned char color);
void gfx_show(); 
void wait_vrt();
void draw_string(int x, int y, const char *s, unsigned char color, int scale);

#endif