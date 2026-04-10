#include "fixed.h"
#include <math.h> // We are allowed to use math.h ONLY during initialization

fixed fix_sin_table[TRIG_TABLE_SIZE];
fixed fix_cos_table[TRIG_TABLE_SIZE];

void init_fixed_math() {
    // Generate the Sine and Cosine tables at startup.
    // This is the ONLY time we use floating-point math.
    for (int i = 0; i < TRIG_TABLE_SIZE; i++) {
        // Convert our 1024-degree integer angle to standard Radians
        double angle_radians = ((double)i * 3.14159265358979323846 * 2.0) / (double)TRIG_TABLE_SIZE;
        
        // Calculate the float value (-1.0 to 1.0) and convert to 16.16 fixed point
        fix_sin_table[i] = FLOAT_TO_FIX(sin(angle_radians));
        fix_cos_table[i] = FLOAT_TO_FIX(cos(angle_radians));
    }
}