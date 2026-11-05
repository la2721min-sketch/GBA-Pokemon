
#include <gba.h>

int main(void)
{
    REG_DISPCNT = MODE_3 | BG2_ENABLE;

    while (1)
    {
        VBlankIntrWait();
        m3_fill(MODE3_FB, RGB5(10, 20, 10), 240 * 160);
    }

    return 0;
}
