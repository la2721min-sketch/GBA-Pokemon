#include <gba.h>
#include <stdbool.h>
#include <stdint.h>

#define W 240
#define H 160
#define TILE 16
#define MAP_W 40
#define MAP_H 30

#define RGB(r,g,b) ((u16)(((r)&31) | (((g)&31)<<5) | (((b)&31)<<10)))

typedef enum {
    T_GRASS = 0, T_PATH, T_TREE, T_WATER, T_TALL_GRASS, T_HOUSE
} Tile;

static const u8 world[MAP_H][MAP_W] = {
    /* Generated in mapTile() for compact source; this array reserves map dimensions. */
    {0}
};

static int playerX = 8, playerY = 8;
static int followerX = 7, followerY = 8;
static int facingX = 0, facingY = 1;
static int cameraX, cameraY;
static int stepsInGrass = 0;
static int dayTick = 0;
static bool inBattle = false;
static int battleMessage = 0;
static int enemyHP = 20;
static int playerHP = 30;
static int enemySpecies = 16;
static int enemyLevel = 3;
static int playerLevel = 5;
static int menuChoice = 0;

static inline void putPixel(int x, int y, u16 color) {
    if (x >= 0 && x < W && y >= 0 && y < H)
        MODE3_FB[y * W + x] = color;
}

static void fillRect(int x, int y, int w, int h, u16 color) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w > W ? W : x + w;
    int y1 = y + h > H ? H : y + h;
    for (int yy = y0; yy < y1; ++yy)
        for (int xx = x0; xx < x1; ++xx)
            putPixel(xx, yy, color);
}

static int tileAt(int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return T_TREE;
    if (x == 0 || y == 0 || x == MAP_W-1 || y == MAP_H-1) return T_TREE;

    /* Small FireRed-inspired starter town layout */
    if ((x >= 4 && x <= 9 && y >= 3 && y <= 6) ||
        (x >= 27 && x <= 33 && y >= 3 && y <= 7)) return T_HOUSE;
    if ((x >= 19 && x <= 24 && y >= 19 && y <= 25)) return T_WATER;
    if (y == 12 || y == 13 || x == 16 || x == 17) return T_PATH;
    if (x >= 20 && x <= 34 && y >= 15 && y <= 26) return T_TALL_GRASS;
    if ((x % 8 == 0 && y % 6 == 0) ||
        (x == 2 && y > 8 && y < 22) ||
        (x == 37 && y > 10 && y < 25)) return T_TREE;
    return T_GRASS;
}

static bool blocked(int x, int y) {
    int t = tileAt(x, y);
    return t == T_TREE || t == T_WATER || t == T_HOUSE;
}

static void drawTile(int t, int sx, int sy, bool night) {
    u16 grass = night ? RGB(3,12,5) : RGB(9,25,10);
    u16 grass2 = night ? RGB(2,9,4) : RGB(7,21,8);
    u16 path = night ? RGB(13,10,6) : RGB(23,18,11);
    u16 tree = night ? RGB(2,8,3) : RGB(3,15,5);
    u16 water = night ? RGB(3,7,17) : RGB(5,15,25);

    switch (t) {
    case T_GRASS:
        fillRect(sx, sy, TILE, TILE, grass);
        fillRect(sx+4, sy+5, 2, 2, grass2);
        fillRect(sx+12, sy+11, 2, 2, grass2);
        break;
    case T_PATH:
        fillRect(sx, sy, TILE, TILE, path);
        fillRect(sx+3, sy+4, 2, 2, RGB(18,14,8));
        break;
    case T_TREE:
        fillRect(sx, sy, TILE, TILE, grass2);
        fillRect(sx+1, sy+1, 14, 12, tree);
        fillRect(sx+6, sy+11, 4, 5, RGB(12,7,3));
        break;
    case T_WATER:
        fillRect(sx, sy, TILE, TILE, water);
        fillRect(sx+3, sy+5, 8, 2, RGB(12,22,29));
        break;
    case T_TALL_GRASS:
        fillRect(sx, sy, TILE, TILE, grass);
        fillRect(sx+2, sy+2, 3, 13, night ? RGB(3,16,6) : RGB(5,19,7));
        fillRect(sx+8, sy+1, 3, 14, night ? RGB(3,14,5) : RGB(5,22,8));
        fillRect(sx+13, sy+5, 2, 10, night ? RGB(3,16,6) : RGB(5,19,7));
        break;
    case T_HOUSE:
        fillRect(sx, sy, TILE, TILE, RGB(22,17,12));
        fillRect(sx, sy, TILE, 5, RGB(20,5,5));
        fillRect(sx+5, sy+7, 6, 9, RGB(9,5,3));
        break;
    }
}

static void drawTrainer(int sx, int sy) {
    fillRect(sx+4, sy, 8, 5, RGB(25,18,12));
    fillRect(sx+3, sy-1, 10, 3, RGB(25,4,4));
    fillRect(sx+4, sy+5, 8, 7, RGB(5,8,25));
    fillRect(sx+3, sy+12, 4, 4, RGB(5,5,5));
    fillRect(sx+9, sy+12, 4, 4, RGB(5,5,5));
}

static void drawFollower(int sx, int sy) {
    /* Temporary generic partner creature sprite; replaceable with original art. */
    fillRect(sx+4, sy+5, 8, 8, RGB(29,25,4));
    fillRect(sx+3, sy+1, 4, 6, RGB(29,25,4));
    fillRect(sx+9, sy+1, 4, 6, RGB(29,25,4));
    fillRect(sx+3, sy+8, 2, 2, RGB(25,4,4));
    fillRect(sx+11, sy+8, 2, 2, RGB(25,4,4));
}

static void updateCamera(void) {
    cameraX = playerX*TILE - W/2 + TILE/2;
    cameraY = playerY*TILE - H/2 + TILE/2;
    if (cameraX < 0) cameraX = 0;
    if (cameraY < 0) cameraY = 0;
    if (cameraX > MAP_W*TILE-W) cameraX = MAP_W*TILE-W;
    if (cameraY > MAP_H*TILE-H) cameraY = MAP_H*TILE-H;
}

static void drawWorld(void) {
    bool night = ((dayTick / 900) % 2) != 0;
    for (int sy = 0; sy <= H/TILE+1; ++sy) {
        for (int sx = 0; sx <= W/TILE+1; ++sx) {
            int tx = cameraX/TILE + sx;
            int ty = cameraY/TILE + sy;
            drawTile(tileAt(tx, ty), tx*TILE-cameraX, ty*TILE-cameraY, night);
        }
    }
    drawFollower(followerX*TILE-cameraX, followerY*TILE-cameraY);
    drawTrainer(playerX*TILE-cameraX, playerY*TILE-cameraY);
    if (night) {
        /* Simple night tint overlay using sparse dark pixels. */
        for (int y=0; y<H; y+=3)
            for (int x=0; x<W; x+=3)
                putPixel(x,y,RGB(1,2,5));
    }
}

static void drawBattle(void) {
    fillRect(0,0,W,H,RGB(9,17,25));
    fillRect(0,75,W,85,RGB(12,22,10));
    fillRect(0,102,W,58,RGB(5,15,7));
    /* Arena platforms */
    fillRect(145,48,70,8,RGB(17,20,14));
    fillRect(18,94,76,8,RGB(17,20,14));
    /* Placeholder wild creature */
    fillRect(169,24,25,24,RGB(19,20,24));
    fillRect(176,18,11,12,RGB(19,20,24));
    /* Player partner */
    fillRect(45,69,25,24,RGB(27,20,5));
    fillRect(49,63,8,10,RGB(27,20,5));
    /* Status panels */
    fillRect(8,8,112,35,RGB(31,31,27));
    fillRect(122,108,110,43,RGB(31,31,27));
    fillRect(14,14,100,23,RGB(3,3,3));
    fillRect(128,114,98,30,RGB(3,3,3));
    fillRect(14,14,enemyHP*4,3,RGB(4,25,5));
    fillRect(128,114,playerHP*3,3,RGB(4,25,5));
    /* Battle action menu */
    fillRect(133,119,91,21,RGB(8,10,17));
    fillRect(138,124,37,11,RGB(20,20,25));
    fillRect(183,124,35,11,RGB(20,20,25));
    if (menuChoice == 0) fillRect(135,122,3,3,RGB(31,31,31));
    else fillRect(180,122,3,3,RGB(31,31,31));
}

static void beginBattle(void) {
    inBattle = true;
    enemyHP = 20;
    playerHP = 30;
    enemySpecies = 16;
    enemyLevel = 2 + (playerLevel / 2);
    battleMessage = 0;
    menuChoice = 0;
}

static void battleInput(u16 keys) {
    if (keys & (KEY_LEFT | KEY_RIGHT)) menuChoice ^= 1;
    if (keys & KEY_A) {
        if (menuChoice == 0) {
            enemyHP -= 8;
            battleMessage = 1;
            if (enemyHP <= 0) {
                enemyHP = 0;
                playerLevel++;
                if (playerLevel > 500) playerLevel = 500;
                inBattle = false;
            } else {
                playerHP -= 4;
                if (playerHP <= 0) inBattle = false;
            }
        } else {
            /* Prototype capture action: guaranteed catch for demo. */
            inBattle = false;
        }
    }
    if (keys & KEY_B) inBattle = false;
}

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);
    REG_DISPCNT = MODE_3 | BG2_ENABLE;

    while (1) {
        VBlankIntrWait();
        scanKeys();
        u16 down = keysDown();
        u16 held = keysHeld();

        dayTick++;

        if (inBattle) {
            battleInput(down);
            drawBattle();
            continue;
        }

        int dx = 0, dy = 0;
        if (held & KEY_UP) dy = -1;
        else if (held & KEY_DOWN) dy = 1;
        else if (held & KEY_LEFT) dx = -1;
        else if (held & KEY_RIGHT) dx = 1;

        if (dx || dy) {
            facingX = dx; facingY = dy;
            int nx = playerX + dx;
            int ny = playerY + dy;
            if (!blocked(nx, ny)) {
                playerX = nx;
                playerY = ny;
                followerX = playerX - dx;
                followerY = playerY - dy;
                if (tileAt(playerX, playerY) == T_TALL_GRASS) {
                    stepsInGrass++;
                    if (stepsInGrass >= 12) {
                        stepsInGrass = 0;
                        beginBattle();
                    }
                }
            }
        }

        if (down & KEY_A) {
            int tx = playerX + facingX;
            int ty = playerY + facingY;
            if (tileAt(tx, ty) == T_TALL_GRASS) beginBattle();
        }

        updateCamera();
        drawWorld();
    }
    return 0;
}
