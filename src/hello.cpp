#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#undef __STRICT_ANSI__
#define __DJGPP__ 2

#include <cstdio>
#include <time.h>    
#include <dos.h>
#include <dpmi.h>

#include "graphics.h"
#include "keyboard.h" // Include custom keyboard handler

int main() {
    printf("Initializing 486DX Game Engine...\n");
    
    if (!init_graphics()) {
        printf("Fatal Error: Could not initialize graphics memory.\n");
        return 1;
    }

    // Initialize keyboard AFTER graphics, but BEFORE entering the loop
    if (!init_keyboard()) {
        printf("Fatal Error: Could not hook INT 9.\n");
        shutdown_graphics();
        return 1;
    }

    set_mode13h(); 
    
    bool running = true;
    
    uclock_t start_time = uclock();
    int frames = 0;
    int current_fps = 0;
    char fps_string[16];

    int player_x = 160;
    int player_y = 100;

    // Main Game Loop
    while (running) {
        frames++;
        uclock_t current_time = uclock();
        if (current_time - start_time >= UCLOCKS_PER_SEC) {
            current_fps = frames;
            frames = 0;
            start_time = current_time;
        }

        // --- 1. NON-BLOCKING INPUT POLLING ---
        if (keys[KEY_ESC]) {
            running = false;
        }
        
        // Multiple keys can be processed simultaneously
        if (keys[KEY_UP])    player_y -= 2;
        if (keys[KEY_DOWN])  player_y += 2;
        if (keys[KEY_LEFT])  player_x -= 2;
        if (keys[KEY_RIGHT]) player_x += 2;

        // Keep player on screen
        if (player_x < 0) player_x = 0;
        if (player_x > 310) player_x = 310;
        if (player_y < 0) player_y = 0;
        if (player_y > 190) player_y = 190;

        // --- 2. RENDER PHASE ---
        clear_screen_mode13h(0); 

        // Draw Player (a simple colored square for now)
        for(int py = 0; py < 10; py++) {
            for(int px = 0; px < 10; px++) {
                put_pixel(player_x + px, player_y + py, 10); // Green
            }
        }

        sprintf(fps_string, "FPS: %d", current_fps);
        draw_string(2, 2, fps_string, 15, 1); 

        if (keys[KEY_SPACE]) {
            draw_string(100, 180, "BRAKING / FIRING", 12, 1); 
        }

        update_screen(); 
    }

    // Teardown
    restore_text_mode(); 
    shutdown_keyboard(); // MUST call before exit
    shutdown_graphics();

    printf("Engine shut down cleanly.\n");
    return 0;
}