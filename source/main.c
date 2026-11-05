
#include <gba.h>
#include <stdbool.h>

#define SCREEN_W 240
#define SCREEN_H 160

static void pixel(int x, int y, u16 color)
{
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H)
        MODE3_FB[y * SCREEN_W + x] = color;
}

static void rectangle(int x, int y, int w, int h, u16 color)
{
    for (int py = y; py < y + h; py++)
        for (int px = x; px < x + w; px++)
            pixel(px, py, color);
}

static void drawWorld(int px, int py)
{
    for (int y = 0; y < SCREEN_H; y++)
        for (int x = 0; x < SCREEN_W; x++)
            pixel(x, y, RGB5(8, 22, 8));

    rectangle(0, 100, SCREEN_W, 35, RGB5(22, 16, 8));
    rectangle(90, 0, 35, SCREEN_H, RGB5(22, 16, 8));

    /* Trees */
    for (int x = 15; x < 225; x += 30)
    {
        rectangle(x, 25, 16, 16, RGB5(4, 18, 3));
        rectangle(x + 5, 41, 6, 7, RGB5(15, 8, 3));
    }

    /* Player */
    rectangle(px, py, 8, 12, RGB5(5, 5, 31));
    rectangle(px + 1, py - 4, 6, 5, RGB5(31, 23, 15));

    /* Following Pokemon placeholder */
    rectangle(px - 12, py + 3, 7, 7, RGB5(31, 28, 5));
}

int main(void)
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    REG_DISPCNT = MODE_3 | BG2_ENABLE;

    int px = 116;
    int py = 78;

    while (1)
    {
        VBlankIntrWait();
        scanKeys();

        u16 keys = keysHeld();

        if (keys & KEY_UP)    py--;
        if (keys & KEY_DOWN)  py++;
        if (keys & KEY_LEFT)  px--;
        if (keys & KEY_RIGHT) px++;

        if (px < 0) px = 0;
        if (py < 4) py = 4;
        if (px > 232) px = 232;
        if (py > 148) py = 148;

        drawWorld(px, py);
    }

    return 0;
}
