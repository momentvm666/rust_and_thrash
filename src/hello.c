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
    char debug_str[32] = "FPS: Calculating...";
    int frames_this_second = 0;

    while (running) {
        t_now = uclock();
        
        // Update the counter exactly once per second
        if (t_now - t_last >= UCLOCKS_PER_SEC) {
            sprintf(debug_str, "FPS: %d", frames_this_second);
            frames_this_second = 0;
            
            // Realign the timer. We do not just set t_last = t_now to prevent 
            // drift if the frame processing pushed us slightly over the 1-second mark.
            t_last += UCLOCKS_PER_SEC; 
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
        
        frames_this_second++;
    }

    shutdown_keyboard();
    shutdown_graphics();
    
    return 0;
}
