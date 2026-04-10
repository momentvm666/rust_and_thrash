#include "graphics.h"
#include "road.h"
#include <string.h>

#define HORIZON 100

int road_w_table[200];
int curve_mult_table[200];
int rumble_state_table[200];

/**
 * fast_fill: 32-bit optimized memory filler.
 * Uses rep stosl for dword-aligned blocks and rep stosb for the remainder.
 */
static inline void fast_fill(unsigned char *dest, unsigned char color, int count) {
    if (count <= 0) return;

    unsigned int pattern = color | (color << 8) | (color << 16) | (color << 24);
    int d0, d1, d2;

    __asm__ __volatile__ (
        "movl %%ecx, %%ebx\n\t"
        "shrl $2, %%ecx\n\t"    // count / 4
        "andl $3, %%ebx\n\t"    // count % 4
        "cld\n\t"
        "rep stosl\n\t"         // Write dwords
        "movl %%ebx, %%ecx\n\t"
        "rep stosb\n\t"         // Write remaining bytes
        : "=&D" (d0), "=&c" (d1), "=&a" (d2)
        : "0" (dest), "1" (count), "2" (pattern)
        : "ebx", "memory", "cc"
    );
}

/**
 * Helper to handle segment clipping and call the fast filler.
 */
static inline void draw_seg(unsigned char *line_ptr, int start, int end, unsigned char color) {
    if (start < 0) start = 0;
    if (end > 320) end = 320;
    if (end > start) {
        fast_fill(line_ptr + start, color, end - start);
    }
}

void init_road() {
    for (int y = HORIZON; y < 200; y++) {
        float depth_linear = (float)(y - HORIZON);
        road_w_table[y] = (int)(depth_linear * 1.6f); 
        
        float z_inv = depth_linear / 100.0f;
        curve_mult_table[y] = (int)(z_inv * z_inv * 1500.0f);
        rumble_state_table[y] = (int)(100.0f / (depth_linear + 1.0f));
    }
}

void draw_road(fixed cam_x, fixed track_curve, fixed track_pos) {
    // Fill the sky: 32,000 bytes. This is the largest fill and benefits most 
    // from 32-bit rep stosl.
    fast_fill(screen_buffer, 1, 32000);

    const int pos_int = FIX_TO_INT(track_pos);
    const int cam_int = FIX_TO_INT(cam_x);
    const int curve_val = FIX_TO_INT(track_curve);

    for (int y = HORIZON; y < 200; y++) {
        int curve_x = (curve_val * curve_mult_table[y]) >> 7;
        int center_x = 160 + curve_x - cam_int;
        
        int road_w = road_w_table[y];
        int r_w = (road_w >> 3) + 1; // Rumble width

        int x1 = center_x - road_w;
        int x2 = center_x + road_w;

        // Determine colors based on distance/position
        int is_rumble = ((pos_int + rumble_state_table[y]) >> 2) & 1;
        unsigned char r_c = is_rumble ? 4 : 15;  // Red vs White
        unsigned char g_c = is_rumble ? 2 : 10;  // Dark green vs Light green

        unsigned char *line_ptr = screen_buffer + row_offset[y];

        // Segmented rendering (Zero Overdraw logic)
        // [Grass] [Rumble] [Road] [Rumble] [Grass]
        
        int l_rumble_end = x1 + r_w;
        int r_rumble_start = x2 - r_w;

        draw_seg(line_ptr, 0, x1, g_c);               // Left Grass
        draw_seg(line_ptr, x1, l_rumble_end, r_c);    // Left Rumble
        draw_seg(line_ptr, l_rumble_end, r_rumble_start, 8); // Road
        draw_seg(line_ptr, r_rumble_start, x2, r_c);   // Right Rumble
        draw_seg(line_ptr, x2, 320, g_c);             // Right Grass
    }
}