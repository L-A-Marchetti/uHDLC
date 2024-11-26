#include "../include/uHDLC.h"

#include <stdint.h>
#include <string.h>

int main()
{
    const char* data = "Hello,~World";
    uHDLC frame = uHDLC_encode(HDLC_ADDR_NONE, HDLC_I, (const uint8_t *)data, strlen(data));
    uHDLC_print(&frame);
    const uint8_t frame_r[] = {
        0x7E,    // Flag
        0x00,    // Addr
        0x00,    // Control
        0x48,    // 'H'
        0x65,    // 'e'
        0x6C,    // 'l'
        0x6C,    // 'l'
        0x6F,    // 'o'
        0x2C,    // ','
        126,     // '~'
        0x57,   // 'W'
        0x6F,   // 'o'
        0x72,   // 'r'
        0x6C,   // 'l'
        0x64,   // 'd'
        0xbe,   // Fcs[1/2]
        0x5b,   // Fcs[2/2]
        0x7E    // Flag
    };
    uHDLC decode = uHDLC_decode(frame_r, sizeof(frame_r)/sizeof(frame_r[0]));
    uHDLC_print(&decode);
    return 0;
}