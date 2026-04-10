#include <dpmi.h>
#include <go32.h>
#include <string.h>
#include <stdlib.h>
#include <sys/nearptr.h>
#include <pc.h>
#include "graphics.h"
#include "small_font.h" 

unsigned char *screen_buffer = NULL;
unsigned char *VGA_MEMORY = NULL;
int row_offset[200];

bool init_graphics() {
    screen_buffer = (unsigned char *)malloc(64000);
    if (!screen_buffer) return false;

    if (!__djgpp_nearptr_enable()) {
        free(screen_buffer);
        return false;
    }

    VGA_MEMORY = (unsigned char *)(0xA0000 + __djgpp_conventional_base);
    memset(screen_buffer, 0, 64000);

    for (int i = 0; i < 200; i++) {
        row_offset[i] = i * 320;
    }
    
    return true;
}

void shutdown_graphics() {
    if (screen_buffer != NULL) free(screen_buffer);
    __djgpp_nearptr_disable();
}

void put_pixel(int x, int y, unsigned char color) {
    if ((unsigned)x < 320 && (unsigned)y < 200) {
        screen_buffer[row_offset[y] + x] = color;
    }
}

void blit_full_screen() {
    int d0, d1, d2;
    __asm__ __volatile__ (
        "cld\n\t"
        "rep movsl\n\t"
        : "=&S" (d0), "=&D" (d1), "=&c" (d2)
        : "0" (screen_buffer), "1" (VGA_MEMORY), "2" (16000)
        : "memory", "cc"
    );
}

void draw_string(int x, int y, const char *s, unsigned char color, int scale) {
    int current_x = x;
    for (int i = 0; s[i] != '\0'; ++i) {
        if (s[i] < 32 || s[i] > 127) continue;
        int idx = s[i] - 32;
        for (int row = 0; row < 5; ++row) {
            unsigned char data = small_font[idx][row];
            for (int col = 0; col < 3; ++col) {
                if ((data >> (7 - col)) & 0x01) { 
                    for (int sy = 0; sy < scale; ++sy) {
                        for (int sx = 0; sx < scale; ++sx) {
                            put_pixel(current_x + (col * scale) + sx, y + (row * scale) + sy, color);
                        }
                    }
                }
            }
        }
        current_x += 4 * scale; 
    }
}

void wait_vrt() {
    while (inportb(0x3DA) & 8);
    while (!(inportb(0x3DA) & 8));
}

void set_mode13h() {
    __dpmi_regs regs;
    regs.x.ax = 0x0013; 
    __dpmi_int(0x10, &regs); 
}

void restore_text_mode() {
    __dpmi_regs regs;
    regs.x.ax = 0x0003; 
    __dpmi_int(0x10, &regs);
}