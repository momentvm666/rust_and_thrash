#ifndef FIXED_H
#define FIXED_H

// A 16.16 fixed point number is just a standard 32-bit signed integer.
// The top 16 bits represent the whole number, the bottom 16 bits represent the fraction.
typedef int fixed;

#define FIX_SHIFT 16
#define FIX_SCALE 65536

// Macros for conversion
// IMPORTANT: Only use FLOAT_TO_FIX for pre-calculating constants at compile time
// or during level initialization. NEVER use it inside the game loop.
#define INT_TO_FIX(x)   ((x) << FIX_SHIFT)
#define FIX_TO_INT(x)   ((x) >> FIX_SHIFT)
#define FLOAT_TO_FIX(x) ((fixed)((x) * FIX_SCALE))

// Inline assembly 16.16 multiplication
inline fixed fix_mul(fixed a, fixed b) {
    fixed result;
    __asm__ __volatile__ (
        "imull %2\n\t"                // Multiply EAX by 'b'. 64-bit result goes into EDX:EAX
        "shrdl $16, %%edx, %%eax\n\t" // Shift EDX:EAX right by 16. The middle 32 bits land in EAX.
        : "=a" (result)               // Output: result goes into EAX
        : "a" (a), "rm" (b)           // Inputs: EAX = a, Any Register/Memory = b
        : "edx", "cc"                 // Clobbers: EDX and condition codes
    );
    return result;
}

// Inline assembly 16.16 division
inline fixed fix_div(fixed a, fixed b) {
    fixed result;
    __asm__ __volatile__ (
        "movl %%eax, %%edx\n\t"       // Copy 'a' to EDX
        "sarl $16, %%edx\n\t"         // Arithmetic shift right 16: EDX gets the sign-extended whole number part
        "shll $16, %%eax\n\t"         // Shift left 16: EAX gets the fractional part, bottom fills with 0s
        "idivl %2\n\t"                // Divide the 64-bit integer in EDX:EAX by 'b'. Result in EAX.
        : "=a" (result)               // Output: result goes into EAX
        : "a" (a), "rm" (b)           // Inputs: EAX = a, Any Register/Memory = b
        : "edx", "cc"                 // Clobbers: EDX and condition codes
    );
    return result;
}

// Fixed-point absolute value
inline fixed fix_abs(fixed a) {
    return (a < 0) ? -a : a;
}

// Trigonometry lookup tables (Angles 0 to 1023)
#define TRIG_TABLE_SIZE 1024
#define TRIG_MASK 1023

extern fixed fix_sin_table[TRIG_TABLE_SIZE];
extern fixed fix_cos_table[TRIG_TABLE_SIZE];

void init_fixed_math();

// Fast Sine/Cosine using bitwise AND wrapping (no expensive modulo % operator)
inline fixed fix_sin(int angle) {
    return fix_sin_table[angle & TRIG_MASK];
}

inline fixed fix_cos(int angle) {
    return fix_cos_table[angle & TRIG_MASK];
}

#endif // FIXED_H