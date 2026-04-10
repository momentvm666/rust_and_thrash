#ifndef GRAPHICS_H
#define GRAPHICS_H

bool init_graphics();
void shutdown_graphics();

void put_pixel(int x, int y, unsigned char color);

// Added scale parameter with default value of 1
void draw_char(int x, int y, char c, unsigned char color, int scale = 1);
void draw_string(int x, int y, const char *s, unsigned char color, int scale = 1);

void clear_screen_mode13h(unsigned char color);
void update_screen();

void set_mode13h();
void restore_text_mode();

#endif // GRAPHICS_H