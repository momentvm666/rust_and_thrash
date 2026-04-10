#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

extern unsigned char *screen_buffer;
extern int row_offset[200]; 

bool init_graphics();
void shutdown_graphics();
void put_pixel(int x, int y, unsigned char color);
void draw_string(int x, int y, const char *s, unsigned char color, int scale);

void blit_full_screen(); 
void wait_vrt();
void set_mode13h();
void restore_text_mode();

#endif