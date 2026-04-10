#ifndef KEYBOARD_H
#define KEYBOARD_H

// volatile is strictly required so the compiler does not optimize away
// checks to this array in your main game loop.
extern volatile char keys[128];

// Common XT Scan Codes
#define KEY_ESC     1
#define KEY_SPACE   57
#define KEY_UP      72
#define KEY_LEFT    75
#define KEY_RIGHT   77
#define KEY_DOWN    80

bool init_keyboard();
void shutdown_keyboard();

#endif // KEYBOARD_H