#include <pc.h>
#include <dpmi.h>
#include <go32.h>
#include <stdbool.h>
#include "keyboard.h"

volatile char keys[128] = {0};

static _go32_dpmi_seginfo old_handler, new_handler;

// The actual Interrupt Service Routine (ISR)
void keyboard_isr() {
    // Read the scan code from the keyboard data port
    unsigned char scancode = inportb(0x60);
    
    // If the top bit is 0, it's a "make" code (key pressed)
    if (scancode < 128) {
        keys[scancode] = 1; 
    } 
    // If the top bit is 1, it's a "break" code (key released)
    else {
        // Strip the top bit to get the original scan code index
        keys[scancode & 0x7F] = 0; 
    }
    
    // Acknowledge the interrupt to the Programmable Interrupt Controller (PIC)
    // Send End of Interrupt (EOI) signal (0x20) to the Master PIC command port (0x20)
    outportb(0x20, 0x20); 
}
// Dummy function used strictly to calculate the length of the ISR for memory locking
void keyboard_isr_end() {}

bool init_keyboard() {
    // 1. Lock the array and the code in memory to prevent fatal page faults
    _go32_dpmi_lock_data((void *)keys, sizeof(keys));
    _go32_dpmi_lock_code((void *)keyboard_isr, (unsigned long)keyboard_isr_end - (unsigned long)keyboard_isr);

    // 2. Save the original BIOS keyboard handler
    _go32_dpmi_get_protected_mode_interrupt_vector(0x09, &old_handler);

    // 3. Set up the new handler
    new_handler.pm_offset = (unsigned long)keyboard_isr;
    new_handler.pm_selector = _go32_my_cs();
    
    // 4. Allocate a DPMI wrapper so the real-mode interrupt can safely call our 32-bit C function
    if (_go32_dpmi_allocate_iret_wrapper(&new_handler) != 0) {
        return false;
    }

    // 5. Install the new handler
    _go32_dpmi_set_protected_mode_interrupt_vector(0x09, &new_handler);
    
    return true;
}

void shutdown_keyboard() {
    // Restore the original BIOS handler before the program exits
    // If you fail to do this, DOS will crash upon returning to the command prompt.
    _go32_dpmi_set_protected_mode_interrupt_vector(0x09, &old_handler);
    _go32_dpmi_free_iret_wrapper(&new_handler);
}