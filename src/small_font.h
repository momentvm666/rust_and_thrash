#ifndef SMALL_FONT_H
#define SMALL_FONT_H

// 8x8 pixel font data (ASCII characters 32-127)
// Each character is 8 bytes, where each bit represents a pixel.
// 1 = pixel on, 0 = pixel off (transparent)
extern const unsigned char small_font[96][5];

#endif // SMALL_FONT_H
