
#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define SCREEN_W 400
#define SCREEN_H 240
#define TILE 16
#define MAP_W 40
#define MAP_H 30

#define PARTY_SIZE 6

typedef struct {
    float x, y;
    float targetX, targetY;
} Character;

typedef struct {
    int species;
    int level;
    int hp;
    int maxHP;
} Pokemon;

static Character player = {8, 8, 8, 8};
static Character follower = {7, 8, 7, 8};

static Pokemon party[PARTY_SIZE] = {
    {25, 5, 20, 20},
    {1, 5, 20, 20},
    {4, 5, 20, 20},
    {7, 5, 20, 20},
    {133, 5, 20, 20},
    {39, 5, 20, 20}
};

static int cameraX = 0;
static int cameraY = 0;
static int npcX = 13;
static int npcY = 8;
static int messageTimer = 0;

static const u32 GRASS  = 0xFF55A84AFF;
static const u32 DARK_GRASS = 0xFF39863AFF;
static const u32 PATH   = 0xFFBDA16AFF;
static const u32 TREE   = 0xFF17652EFF;
static const u32 WATER  = 0xFFCC8844FF;
static const u32 HOUSE  = 0xFFAA5544FF;
static const u32 ROOF   = 0xFF663333FF;

static int tileAt(int x, int y)
{
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H)
        return 1;

    if (x == 0 || y == 0 || x == MAP_W - 1 || y == MAP_H - 1)
        return 1;

    /* Water pond */
    if (x >= 24 && x <= 29 && y >= 5 && y <= 9)
        return 3;

    /* House */
    if (x >= 5 && x <= 9 && y >= 3 && y <= 6)
        return 4;

    /* Main road */
    if (y == 12 || y == 13 || x == 18)
        return 2;

    /* Trees */
    if ((x % 7 == 0 && y % 5 == 0) ||
        (x == 3 && y > 15 && y < 23) ||
        (x == 35 && y > 12 && y < 25))
        return 1;

    /* Tall grass */
    if (x >= 20 && x <= 33 && y >= 15 && y <= 24)
        return 5;

    return 0;
}

static bool blocked(int x, int y)
{
    int tile = tileAt(x, y);

    if (tile == 1 || tile == 3 || tile == 4)
        return true;

    if (x == npcX && y == npcY)
        return true;

    return false;
}

static void rect(float x, float y, float w, float h, u32 color)
{
    C2D_DrawRectSolid(x, y, 0, w, h, color);
}

static void drawTile(int type, float x, float y)
{
    switch (type)
    {
        case 0:
            rect(x, y, TILE, TILE, GRASS);
            rect(x + 4, y + 5, 2, 2, DARK_GRASS);
            rect(x + 12, y + 11, 2, 2, DARK_GRASS);
            break;

        case 1:
            rect(x, y, TILE, TILE, DARK_GRASS);
            rect(x + 2, y + 1, 12, 11, TREE);
            rect(x + 6, y + 11, 4, 5, 0xFF754522);
            break;

        case 2:
            rect(x, y, TILE, TILE, PATH);
            rect(x + 3, y + 4, 2, 2, 0xFFAA8B59);
            break;

        case 3:
            rect(x, y, TILE, TILE, WATER);
            rect(x + 3, y + 4, 8, 2, 0xFF77CFFFFF);
            break;

        case 4:
            rect(x, y, TILE, TILE, HOUSE);
            rect(x, y, TILE, 5, ROOF);
            rect(x + 5, y + 7, 6, 9, 0xFF553322);
            break;

        case 5:
            rect(x, y, TILE, TILE, GRASS);
            rect(x + 3, y + 2, 3, 12, 0xFF26833AFF);
            rect(x + 9, y + 1, 3, 13, 0xFF287C35FF);
            rect(x + 13, y + 5, 2, 9, 0xFF2D923FFF);
            break;
    }
}

static void drawPlayer(float x, float y)
{
    /* Simple pixel-art trainer */
    rect(x + 4, y + 1, 8, 5, 0xFFFFD6A0);
    rect(x + 3, y, 10, 3, 0xFFCC3333);
    rect(x + 4, y + 6, 8, 6, 0xFF3366CC);
    rect(x + 3, y + 12, 4, 4, 0xFF333333);
    rect(x + 9, y + 12, 4, 4, 0xFF333333);
}

static void drawFollower(float x, float y)
{
    /* Pikachu-style placeholder follower */
    rect(x + 4, y + 5, 8, 8, 0xFFFFDD33);
    rect(x + 3, y + 2, 4, 5, 0xFFFFDD33);
    rect(x + 9, y + 2, 4, 5, 0xFFFFDD33);
    rect(x + 2, y + 1, 2, 4, 0xFF333333);
    rect(x + 12, y + 1, 2, 4, 0xFF333333);
    rect(x + 3, y + 9, 2, 2, 0xFFCC3333);
    rect(x + 11, y + 9, 2, 2, 0xFFCC3333);
}

static void drawNPC(float x, float y)
{
    rect(x + 4, y + 1, 8, 5, 0xFFFFD6A0);
    rect(x + 3, y, 10, 3, 0xFF6633AA);
    rect(x + 4, y + 6, 8, 6, 0xFFEEEEEE);
    rect(x + 3, y + 12, 4, 4, 0xFF333333);
    rect(x + 9, y + 12, 4, 4, 0xFF333333);
}

static void drawWorld(void)
{
    int startX = cameraX / TILE;
    int startY = cameraY / TILE;

    for (int y = 0; y < 17; y++)
    {
        for (int x = 0; x < 27; x++)
        {
            int tx = startX + x;
            int ty = startY + y;

            drawTile(
                tileAt(tx, ty),
                tx * TILE - cameraX,
                ty * TILE - cameraY
            );
        }
    }

    drawNPC(
        npcX * TILE - cameraX,
        npcY * TILE - cameraY
    );

    drawFollower(
        follower.x * TILE - cameraX,
        follower.y * TILE - cameraY
    );

    drawPlayer(
        player.x * TILE - cameraX,
        player.y * TILE - cameraY
    );
}

static void updateCamera(void)
{
    cameraX = (int)(player.x * TILE - SCREEN_W / 2 + TILE / 2);
    cameraY = (int)(player.y * TILE - SCREEN_H / 2 + TILE / 2);

    if (cameraX < 0) cameraX = 0;
    if (cameraY < 0) cameraY = 0;

    int maxX = MAP_W * TILE - SCREEN_W;
    int maxY = MAP_H * TILE - SCREEN_H;

    if (cameraX > maxX) cameraX = maxX;
    if (cameraY > maxY) cameraY = maxY;
}

static void updateFollower(void)
{
    follower.targetX = player.x - 1;
    follower.targetY = player.y;

    follower.x += (follower.targetX - follower.x) * 0.15f;
    follower.y += (follower.targetY - follower.y) * 0.15f;
}

static void showMessage(const char *message)
{
    consoleClear();
    printf("\n\n  %s\n", message);
    messageTimer = 180;
}

static void interact(void)
{
    int px = (int)(player.x + 0.5f);
    int py = (int)(player.y + 0.5f);

    int dx = (npcX > px) ? npcX - px : px - npcX;
    int dy = (npcY > py) ? npcY - py : py - npcY;

    if (dx + dy <= 1)
    {
        showMessage("Hello, Trainer! Welcome!");
    }
    else
    {
        showMessage("Your Pokemon is following you!");
    }
}

int main(void)
{
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C2D_Target *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    consoleClear();
    printf("POKEMON WORLD\n");
    printf("--------------------\n");
    printf("Welcome, Trainer!\n");
    printf("A: Talk / Interact\n");
    printf("D-Pad: Move\n");
    printf("START: Exit\n");

    bool running = true;

    while (aptMainLoop() && running)
    {
        hidScanInput();

        u32 down = hidKeysDown();
        u32 held = hidKeysHeld();

        if (down & KEY_START)
            running = false;

        int dx = 0;
        int dy = 0;

        if (held & KEY_UP) dy = -1;
        if (held & KEY_DOWN) dy = 1;
        if (held & KEY_LEFT) dx = -1;
        if (held & KEY_RIGHT) dx = 1;

        if (dx != 0 || dy != 0)
        {
            int nx = (int)(player.x + dx);
            int ny = (int)(player.y + dy);

            if (!blocked(nx, ny))
            {
                player.x = nx;
                player.y = ny;
            }
        }

        if (down & KEY_A)
            interact();

        updateFollower();
        updateCamera();

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C2D_TargetClear(top, 0xFF000000);
        C2D_SceneBegin(top);

        drawWorld();

        C3D_FrameEnd(0);

        if (messageTimer > 0)
        {
            messageTimer--;
        }
        else
        {
            consoleClear();
            printf("POKEMON WORLD\n");
            printf("--------------------\n");
            printf("A: Talk / Interact\n");
            printf("D-Pad: Move\n");
            printf("START: Exit\n");
            printf("\nParty: %d Pokemon", PARTY_SIZE);
            printf("\nFollower: Pikachu");
        }
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();

    return 0;
}
