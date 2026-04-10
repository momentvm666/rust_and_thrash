#include <stdio.h>
#include <string.h>
#include <time.h>    
#include "graphics.h"
#include "keyboard.h"
#include "fixed.h"
#include "road.h"

int main() {
    if (!init_graphics()) return 1;
    if (!init_keyboard()) return 1;
    
    init_road();
    set_mode13h(); 
    
    bool running = true;
    fixed cam_x = 0;
    fixed distance = 0; 
    fixed speed = 0;
    fixed current_curve = FLOAT_TO_FIX(0.5f); 

    uclock_t t_start, t_end;
    int frame_ms = 0;
    char debug_str[32];
while (running) {
        t_start = uclock();

        // Input and Logic
        if (keys[KEY_ESC]) running = false;
        if (keys[KEY_UP]) speed += 2000; else speed -= 1000;
        if (speed < 0) speed = 0; 
        if (speed > 655360) speed = 655360;
        distance += speed;
        if (keys[KEY_LEFT])  cam_x -= 131072;
        if (keys[KEY_RIGHT]) cam_x += 131072;

        // Render Road
        draw_road(cam_x, current_curve, distance);

        // OPTIONAL: Only update telemetry every 10 frames to save CPU
        static int frame_count = 0;
        if (frame_count++  > 10) {
            sprintf(debug_str, "MS: %d", frame_ms);
            frame_count = 0;
        }
        draw_string(2, 2, debug_str, 15, 1); 
        
        // Use the assembly blit for the WHOLE screen
        //wait_vrt();
        blit_full_screen(); 
        
        t_end = uclock();
        frame_ms = (int)((t_end - t_start) * 1000 / UCLOCKS_PER_SEC);
    }
    restore_text_mode(); 
    shutdown_keyboard();
    shutdown_graphics();
    return 0;
}