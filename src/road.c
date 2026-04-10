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

void draw_road(fixed cam_x, fixed track_curve, fixed track_pos) {
    // 1. FAST SKY CLEAR
    // Set Plane Mask to all 4 planes (0x0F)
    outportw(0x03C4, 0x0F02);
memset(vga_mem + active_page_offset, 1, 80 * HORIZON);

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
        
        int is_rumble = ((pos_int + rumble_state_table[y]) >> 2) & 1;
        unsigned char r_c = is_rumble ? 4 : 15;
        unsigned char g_c = is_rumble ? 2 : 10;

        gfx_draw_span(y, 0, x1, g_c);
        gfx_draw_span(y, x1, x1 + r_w, r_c);
        gfx_draw_span(y, x1 + r_w, x2 - r_w, 8);
        gfx_draw_span(y, x2 - r_w, x2, r_c);
        gfx_draw_span(y, x2, 320, g_c);
    }
}