#include <dpmi.h>
#include <go32.h>
#include <string.h>      // For strlen, memset
#include <stdlib.h>      // For malloc, free
#include <sys/nearptr.h> // For direct hardware memory access
#include <pc.h>          // For inportb (hardware ports)
#include "graphics.h"
#include "small_font.h" 

unsigned char *screen_buffer = NULL;
unsigned char *VGA_MEMORY = NULL;

bool init_graphics() {
    // 1. Allocate 64,000 bytes for the double buffer
    screen_buffer = (unsigned char *)malloc(64000);
    if (screen_buffer == NULL) {
        return false;
    }

    // 2. Disable memory protection limits
    if (!__djgpp_nearptr_enable()) {
        free(screen_buffer);
        screen_buffer = NULL;
        return false;
    }

    // 3. Calculate the linear address that points to physical 0xA0000
    VGA_MEMORY = (unsigned char *)(0xA0000 + __djgpp_conventional_base);
    
    // Clear the buffer initially
    memset(screen_buffer, 0, 64000);
    
    return true;
}

void shutdown_graphics() {
    if (screen_buffer != NULL) {
        free(screen_buffer);
        screen_buffer = NULL;
    }
    // Restore DPMI memory protection
    __djgpp_nearptr_disable();
}

void put_pixel(int x, int y, unsigned char color) {
    if (x >= 0 && x < 320 && y >= 0 && y < 200) {
        if (color != 0) { 
            screen_buffer[y * 320 + x] = color; // Write to buffer, not VGA
        }
    }
}

// Function to draw a single character using the 3x5 font with integer scaling
void draw_char(int x, int y, char c, unsigned char color, int scale) {
    if (c < 32 || c > 127) return;

    int font_char_index = c - 32;

    for (int row = 0; row < 5; ++row) {
        unsigned char line_data = small_font[font_char_index][row];
        for (int col = 0; col < 3; ++col) {
            if ((line_data >> (7 - col)) & 0x01) { 
                // Draw a solid block of scale * scale pixels
                for (int sy = 0; sy < scale; ++sy) {
                    for (int sx = 0; sx < scale; ++sx) {
                        put_pixel(x + (col * scale) + sx, y + (row * scale) + sy, color);
                    }
                }
            }
        }
    }
}

// Function to draw a string with integer scaling
void draw_string(int x, int y, const char *s, unsigned char color, int scale) {
    int current_x = x;
    for (int i = 0; i < strlen(s); ++i) {
        draw_char(current_x, y, s[i], color, scale);
        // Base width is 3, spacing is 1. Total horizontal advance is 4 * scale.
        current_x += 4 * scale; 
    }
}



void clear_screen_mode13h(unsigned char color) {
    memset(screen_buffer, color, 64000); // Fast clear of the buffer
}

// Wait for Vertical Retrace
static void wait_vrt() {
    while (inportb(0x3DA) & 8);
    while (!(inportb(0x3DA) & 8));
}

void update_screen() {
    wait_vrt();
    
    // GCC Inline Assembly for 'rep movsd'
    // Uses dummy output variables to tell the compiler ESI, EDI, and ECX are modified.
    int d0, d1, d2;
    __asm__ __volatile__ (
        "cld\n\t"
        "rep movsd\n\t"
        : "=&S" (d0), "=&D" (d1), "=&c" (d2) // Outputs (clobbered registers)
        : "0" (screen_buffer),               // Input: ESI = buffer
          "1" (VGA_MEMORY),                  // Input: EDI = VGA
          "2" (16000)                        // Input: ECX = 16000 dwords (64kb)
        : "memory", "cc"                     // Clobbers memory and condition codes
    );
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