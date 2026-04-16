#include <string.h>
#include <pc.h>
#include "graphics.h"
#include "road.h"

#define HORIZON 120

int road_w_table[240];
int curve_mult_table[240];
int rumble_state_table[240];

void init_road() {
    for (int y = HORIZON; y < 240; y++) {
        float depth = (float)(y - HORIZON);
        road_w_table[y] = (int)(depth * 1.4f); 
        float z_inv = depth / 120.0f;
        curve_mult_table[y] = (int)(z_inv * z_inv * 2000.0f);
        rumble_state_table[y] = (int)(120.0f / (depth + 1.0f));
    }
}

// road.c

// Define our palette block bases
#define PAL_GRASS    16
#define PAL_ROAD     32
#define PAL_RUMBLE_R 48
#define PAL_RUMBLE_W 64

void draw_road(fixed cam_x, fixed track_curve, fixed track_pos) {
    // FAST SKY CLEAR - Use the fog color's brightest index (e.g., Grass fog at index 16)
    outportw(0x03C4, 0x0F02);
    memset(vga_mem + active_page_offset, PAL_GRASS, 80 * HORIZON);

    const int pos_int = FIX_TO_INT(track_pos);
    const int cam_int = FIX_TO_INT(cam_x);
    const int curve_val = FIX_TO_INT(track_curve);

    for (int y = HORIZON; y < 240; y++) {
        int curve_x = (curve_val * curve_mult_table[y]) >> 7;
        int center_x = 160 + curve_x - cam_int;
        int road_w = road_w_table[y];
        int r_w = (road_w >> 3) + 1;
        int x1 = center_x - road_w;
        int x2 = center_x + road_w;
        
        // Z-Depth Calculation
        // depth goes from 0 (horizon) to 119 (bottom of screen)
        int depth = y - HORIZON;
        
        // Shift right by 3 is mathematically depth / 8.
        // This yields a value from 0 to 14, fitting perfectly into our 16-shade palette.
        int shade = depth >> 3; 

        int is_rumble = ((pos_int + rumble_state_table[y]) >> 2) & 1;
        
        // Base color + Depth shade
        unsigned char g_c = PAL_GRASS + shade;
        unsigned char road_c = PAL_ROAD + shade;
        unsigned char r_c = is_rumble ? (PAL_RUMBLE_W + shade) : (PAL_RUMBLE_R + shade);

        // Ensure you are using the optimized gfx_draw_span_asm function here
        gfx_draw_span(y, 0, x1, g_c);
        gfx_draw_span(y, x1, x1 + r_w, r_c);
        gfx_draw_span(y, x1 + r_w, x2 - r_w, road_c);
        gfx_draw_span(y, x2 - r_w, x2, r_c);
        gfx_draw_span(y, x2, 320, g_c);
    }
}


