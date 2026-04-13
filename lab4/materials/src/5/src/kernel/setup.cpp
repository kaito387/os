#include "asm_utils.h"

extern "C" void setup_kernel() {
    const char studentId[] = "24344064";
    volatile unsigned short *video = (unsigned short *)0xb8000;
    for (int i = 0; studentId[i] != '\0'; ++i) {
        video[i] = (0x03 << 8) | (unsigned char)studentId[i];
    }
    while(1) {

    }
}
