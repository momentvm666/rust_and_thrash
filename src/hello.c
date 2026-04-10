#include <stdio.h>
#include <string.h>
#include <time.h>    
#include "graphics.h"
#include "keyboard.h"
#include "fixed.h"
#include "road.h"

int main() {
    if (!init_graphics(MODE_X_240)) return 1;
    if (!init_keyboard()) return 1;
    
    init_road();
    
    bool running = true;
    fixed cam_x = 0;
    fixed distance = 0; 
    fixed speed = 0;
    fixed current_curve = FLOAT_TO_FIX(0.5f); 

    uclock_t t_last = uclock();
    uclock_t t_now;
    char debug_str[32] = "Frame: 0.0 ms";
    int frame_count = 0;

    while (running) {
        t_now = uclock();
        
        // THE FIX: 64-bit cast prevents overflow during hardware stalls
        int frame_tenths = (int)(((unsigned long long)(t_now - t_last) * 10000ULL) / UCLOCKS_PER_SEC);
        t_last = t_now;

        if (frame_count > 0 && frame_count % 10 == 0) {
            sprintf(debug_str, "Frame: %d.%d ms", frame_tenths / 10, frame_tenths % 10);
        }

        if (keys[KEY_ESC]) running = false;
        if (keys[KEY_UP]) speed += 2000; else speed -= 1000;
        if (speed < 0) speed = 0; 
        if (speed > 655360) speed = 655360;
        distance += speed;
        if (keys[KEY_LEFT])  cam_x -= 131072;
        if (keys[KEY_RIGHT]) cam_x += 131072;

        draw_road(cam_x, current_curve, distance);
        draw_string(10, 10, debug_str, 15, 1);

        gfx_show();
        frame_count++;
    }

    shutdown_keyboard();
    shutdown_graphics();
    
    return 0;
}