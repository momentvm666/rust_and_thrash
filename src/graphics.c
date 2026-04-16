#include <dos.h>
#include <pc.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "small_font.h"

unsigned char *vga_mem = NULL;
int active_page_offset = 19200; 
static int visual_page_offset = 0;
static video_mode_t current_mode;

// graphics.c

// Helper to write a smooth 16-step gradient into the VGA DAC
static void set_color_ramp(int start_index, int fog_r, int fog_g, int fog_b, int true_r, int true_g, int true_b) {
    // Port 0x3C8 sets the index we want to write to
    outportb(0x3C8, start_index);

    for (int i = 0; i < 16; i++) {
        // Interpolate between the fog color (i=0) and the true color (i=15)
        int r = fog_r + ((true_r - fog_r) * i) / 15;
        int g = fog_g + ((true_g - fog_g) * i) / 15;
        int b = fog_b + ((true_b - fog_b) * i) / 15;

        // Port 0x3C9 expects 3 consecutive writes: Red, Green, Blue (0-63)
        outportb(0x3C9, r);
        outportb(0x3C9, g);
        outportb(0x3C9, b);
    }
}

void init_palette() {
    // Define our universal horizon/fog color (A pale, grayish blue)
    int f_r = 35, f_g = 40, f_b = 45;

    // Grass: Indices 16-31 (Fades from Fog to Dark Green)
    set_color_ramp(16, f_r, f_g, f_b,  5, 35,  5);

    // Road: Indices 32-47 (Fades from Fog to Dark Asphalt Gray)
    set_color_ramp(32, f_r, f_g, f_b, 15, 15, 15);

    // Rumble Red: Indices 48-63 (Fades from Fog to Solid Red)
    set_color_ramp(48, f_r, f_g, f_b, 45,  5,  5);

    // Rumble White: Indices 64-79 (Fades from Fog to Bright Off-White)
    set_color_ramp(64, f_r, f_g, f_b, 55, 55, 55);
}

/* * 32-bit inline assembly filler.
 * Writes to the ISA bus are split into 16-bit by the bus controller, 
 * but using 32-bit registers (stosl) saves CPU cycles on the 386DX.
 */
static inline void vga_fast_fill(unsigned char *dest, unsigned char color, int bytes) {
    unsigned int color32 = color | (color << 8) | (color << 16) | (color << 24);
    int dwords = bytes >> 2;
    int rem = bytes & 3;
    __asm__ __volatile__ (
        "cld\n\t"
        "rep stosl\n\t"
        "mov %3, %%ecx\n\t"
        "rep stosb"
        : "+D" (dest)
        : "c" (dwords), "a" (color32), "r" (rem)
        : "memory", "cc"
    );
}

void wait_vrt() {
    while (inportb(0x3DA) & 8);
    while (!(inportb(0x3DA) & 8));
}

static void set_modex_320x240() {
    union REGS regs;
    regs.x.ax = 0x0013;
    int86(0x10, &regs, &regs);

    // 1. Unchain Mode 13h
    outportb(0x03C4, 0x04); outportb(0x03C5, 0x06);
    
    // 2. Select 25.175 MHz dot clock for 60Hz timings
    outportb(0x03C2, 0xE3);
    
    // 3. Unlock CRTC registers 0-7
    outportb(0x03D4, 0x11);
    unsigned char vre = inportb(0x03D5);
    outportb(0x03D5, vre & 0x7F); 

    // 4. THE FIX: Force Byte Mode and disable Doubleword Mode
    outportb(0x03D4, 0x17); outportb(0x03D5, 0xE3); // Mode Control: Byte Mode
    outportb(0x03D4, 0x14); outportb(0x03D5, 0x00); // Underline: Dword off

    // 5. Explicit Vertical Timings (525 Total Scanlines, 480 Visible)
    outportb(0x03D4, 0x06); outportb(0x03D5, 0x0D); // VTotal (525)
    outportb(0x03D4, 0x07); outportb(0x03D5, 0x3E); // Overflow bits
    outportb(0x03D4, 0x09); outportb(0x03D5, 0x41); // Max Scanline (Double scan ON)
    outportb(0x03D4, 0x10); outportb(0x03D5, 0xEA); // VSync Start
    outportb(0x03D4, 0x11); outportb(0x03D5, 0x2C); // VSync End (Keep Protect OFF)
    outportb(0x03D4, 0x12); outportb(0x03D5, 0xDF); // VDisplay End (480 scanlines)
    outportb(0x03D4, 0x15); outportb(0x03D5, 0xE7); // VBlank Start
    outportb(0x03D4, 0x16); outportb(0x03D5, 0x06); // VBlank End

    // 6. Logical line width (40 words = 80 planar bytes)
    outportb(0x03D4, 0x13); outportb(0x03D5, 0x28); 

    // Enable all planes and clear memory
    outportw(0x03C4, 0x0F02); 
    vga_fast_fill(vga_mem, 0, 65535);
}

void gfx_show() {
    unsigned int addr = active_page_offset;
    
    // 1. Queue the address change
    outportb(0x03D4, 0x0C); outportb(0x03D5, (addr >> 8) & 0xFF);
    outportb(0x03D4, 0x0D); outportb(0x03D5, addr & 0xFF);
    
    // 2. Wait for VRT to ensure the hardware latches the queued address
    wait_vrt();
    
    visual_page_offset = active_page_offset;
    active_page_offset = (active_page_offset == 0) ? 19200 : 0;
}

bool init_graphics(video_mode_t mode) {
    current_mode = mode;
    if (!__djgpp_nearptr_enable()) return false;
    vga_mem = (unsigned char *)(0xA0000 + __djgpp_conventional_base);
    if (mode == MODE_X_240) set_modex_320x240();
    return true;
}

void shutdown_graphics() {
    union REGS regs;
    regs.x.ax = 0x0003;
    int86(0x10, &regs, &regs);
    __djgpp_nearptr_disable();
}

void gfx_draw_span(int y, int x1, int x2, unsigned char color) {
    if (y < 0 || y >= 240 || x1 >= x2) return;
    if (x1 < 0) x1 = 0;
    if (x2 > 320) x2 = 320;

    int start_addr = active_page_offset + (y * 80) + (x1 >> 2);
    int end_addr = active_page_offset + (y * 80) + ((x2 - 1) >> 2);
    unsigned char *dest = vga_mem + start_addr;

    int left_mask  = (0x0F << (x1 & 3)) & 0x0F;
    int right_mask = (0x0F >> (3 - ((x2 - 1) & 3))) & 0x0F;

    if (start_addr == end_addr) {
        int mask = left_mask & right_mask;
        outportw(0x03C4, (mask << 8) | 0x02);
        *dest = color;
        return;
    }

    __asm__ __volatile__ (
        "movw $0x03C4, %%dx\n\t"

        // Left edge
        "movb $0x02, %%al\n\t"
        "movb %1, %%ah\n\t"
        "outw %%ax, %%dx\n\t"
        "movb %%bl, (%%edi)\n\t"   // Write color from BL register
        "incl %%edi\n\t"

        // Middle fill
        "movl %3, %%ecx\n\t"
        "subl %2, %%ecx\n\t"
        "decl %%ecx\n\t"
        "jle 1f\n\t"               // Skip if no middle bytes exist

        "movw $0x0F02, %%ax\n\t"
        "outw %%ax, %%dx\n\t"      // Set mask to all 4 planes
        "movb %%bl, %%al\n\t"      // Move color into AL for stosb
        "rep stosb\n\t"            // Blast middle bytes

        "1:\n\t"
        // Right edge
        "movb $0x02, %%al\n\t"
        "movb %4, %%ah\n\t"
        "outw %%ax, %%dx\n\t"
        "movb %%bl, (%%edi)\n\t"   // Write color from BL register

        : "+D" (dest)
        : "m" (left_mask), "m" (start_addr), "m" (end_addr), "m" (right_mask), "b" (color)
        : "eax", "ecx", "edx", "memory"
    );
}


void draw_string(int x, int y, const char *s, unsigned char color, int scale) {
    // Left unchanged for brevity, but can be similarly optimized 
    // by building a local memory buffer and blitting it once per character.
    for (int p = 0; p < 4; p++) {
        outportw(0x03C4, ((1 << p) << 8) | 0x02);
        int cur_x = x;
        for (int i = 0; s[i]; i++) {
            int idx = s[i] - 32;
            for (int r = 0; r < 5; r++) {
                unsigned char bits = small_font[idx][r];
                for (int c = 0; c < 3; c++) {
                    if (!((bits >> (7 - c)) & 1)) continue;
                    for (int sx = 0; sx < scale; sx++) {
                        int px = cur_x + (c * scale) + sx;
                        if ((px & 3) != p) continue;
                        unsigned char *ptr = vga_mem + active_page_offset + (y + r * scale) * 80 + (px >> 2);
                        for (int sy = 0; sy < scale; sy++) ptr[sy * 80] = color;
                    }
                }
            }
            cur_x += 4 * scale;
        }
    }
}
