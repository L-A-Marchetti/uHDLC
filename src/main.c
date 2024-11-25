#include "../include/uHDLC.h"

#include <string.h>

int main()
{
    const char* data = "Hello, World";
    uHDLC frame = uHDLC_frame(HDLC_ADDR_NONE, HDLC_I, (const uint8_t *)data, strlen(data));
    print_uHDLC_frame(&frame);
    return 0;
}