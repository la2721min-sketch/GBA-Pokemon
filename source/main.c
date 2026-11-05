#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define SCREEN_W 400
#define SCREEN_H 240
#define TILE 16
#define MAP_W 32
#define MAP_H 32

#define PARTY_SIZE 6

typedef struct {
    float x, y;
    float speed;
    int direction;
    int moving;
    int frame;
} Player;

typedef struct {
    float x, y;
    int species;
    int level;
    int hp;
    int maxHP;
    int following;
} Pokemon;

static Player player = {
    160,160,1.7f,0,0,0
};

static Pokemon party[PARTY_SIZE] = {
    {145,185,1,12,45,45,1},
    {0,0,4,12,40,40,0},
    {0,0,7,12,42,42,0},
    {0,0,25,12,35,35,0},
    {0,0,133,12,38,38,0},
    {0,0,39,12,50,50,0}
};

static C3D_RenderTarget *topTarget;
static C2D_TextBuf textBuffer;
static C2D_Text text;

static int worldMap[MAP_H][MAP_W];
static int frameCount = 0;

static char message[128] = "Welcome to Pokemon World!";

static void createMap(void)
{
    for(int y=0;y<MAP_H;y++)
    {
        for(int x=0;x<MAP_W;x++)
        {
            worldMap[y][x]=0;

            if(x==0 || y==0 ||
               x==MAP_W-1 || y==MAP_H-1)
                worldMap[y][x]=2;

            if((x==5 || x==6) && y>4 && y<25)
                worldMap[y][x]=1;

            if((x==20 || x==21) && y>5 && y<27)
                worldMap[y][x]=1;
        }
    }

    for(int x=0;x<MAP_W;x++)
    {
        worldMap[15][x]=3;
        worldMap[16][x]=3;
    }
}

static void drawTile(int type,float x,float y)
{
    u32 color;

    switch(type)
    {
        case 0: color=C2D_Color32(70,170,75,255);break;
        case 1: color=C2D_Color32(25,110,40,255);break;
        case 2: color=C2D_Color32(90,65,40,255);break;
        case 3: color=C2D_Color32(190,160,100,255);break;
        default: color=0xFFFFFFFF;break;
    }

    C2D_DrawRectSolid(
        x,y,TILE,TILE,0,color
    );
}

static void drawWorld(void)
{
    float cameraX=player.x-192;
    float cameraY=player.y-112;

    for(int y=0;y<MAP_H;y++)
    {
        for(int x=0;x<MAP_W;x++)
        {
            drawTile(
                worldMap[y][x],
                x*TILE-cameraX,
                y*TILE-cameraY
            );
        }
    }
}

static void drawPlayer(void)
{
    float x=player.x-(player.x-192);
    float y=player.y-(player.y-112);

    C2D_DrawRectSolid(
        x-5,y-8,10,12,0,
        C2D_Color32(220,70,50,255)
    );

    C2D_DrawRectSolid(
        x-5,y-12,10,5,0,
        C2D_Color32(40,50,140,255)
    );

    C2D_DrawCircleSolid(
        x,y-15,0,4,
        C2D_Color32(240,190,145,255)
    );
}

static void drawFollower(void)
{
    Pokemon *p=&party[0];

    float x=p->x-(player.x-192);
    float y=p->y-(player.y-112);

    C2D_DrawCircleSolid(
        x,y,0,7,
        C2D_Color32(230,90,60,255)
    );

    C2D_DrawCircleSolid(
        x,y-2,0,4,
        C2D_Color32(255,210,180,255)
    );
}

static void updateFollower(void)
{
    Pokemon *p=&party[0];

    float targetX=player.x;
    float targetY=player.y+22;

    p->x+=(targetX-p->x)*0.12f;
    p->y+=(targetY-p->y)*0.12f;
}

static void updatePlayer(void)
{
    circlePosition circle;
    hidCircleRead(&circle);

    u32 keys=hidKeysHeld();

    float dx=circle.dx/128.0f;
    float dy=-circle.dy/128.0f;

    if(keys&KEY_LEFT) dx-=1;
    if(keys&KEY_RIGHT) dx+=1;
    if(keys&KEY_UP) dy-=1;
    if(keys&KEY_DOWN) dy+=1;

    float length=sqrtf(dx*dx+dy*dy);

    player.moving=length>0.15f;

    if(length>1)
    {
        dx/=length;
        dy/=length;
    }

    if(player.moving)
    {
        player.x+=dx*player.speed;
        player.y+=dy*player.speed;
        player.frame++;

        if(fabsf(dx)>fabsf(dy))
            player.direction=dx>0?1:3;
        else
            player.direction=dy>0?2:0;
    }

    if(player.x<16) player.x=16;
    if(player.y<16) player.y=16;

    if(player.x>MAP_W*TILE-16)
        player.x=MAP_W*TILE-16;

    if(player.y>MAP_H*TILE-16)
        player.y=MAP_H*TILE-16;

    updateFollower();
}

static void drawBottomScreen(void)
{
    C2D_TargetClear(
        NULL,C2D_Color32(35,45,65,255)
    );

    C2D_SceneBegin(NULL);

    C2D_DrawRectSolid(
        10,10,300,100,0,
        C2D_Color32(240,240,240,255)
    );

    C2D_DrawText(
        &text,C2D_AlignLeft,
        20,20,0,0.6f,0.6f,
        C2D_Color32(20,20,20,255)
    );

    C2D_DrawRectSolid(
        10,125,300,90,0,
        C2D_Color32(60,90,65,255)
    );
}

int main(void)
{
    gfxInitDefault();

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    topTarget=C2D_CreateScreenTarget(
        GFX_TOP,GFX_LEFT
    );

    consoleInit(GFX_BOTTOM,NULL);

    createMap();

    while(aptMainLoop())
    {
        hidScanInput();

        u32 down=hidKeysDown();

        if(down&KEY_START)
            break;

        updatePlayer();

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C2D_TargetClear(
            topTarget,
            C2D_Color32(100,180,240,255)
        );

        C2D_SceneBegin(topTarget);

        drawWorld();
        drawFollower();
        drawPlayer();

        C3D_FrameEnd(0);

        consoleClear();

        printf("POKEMON WORLD GBA EDITION\n");
        printf("=========================\n\n");
        printf("CIRCLE PAD: MOVE\n");
        printf("D-PAD: MOVE\n");
        printf("START: EXIT\n\n");
        printf("PARTY: 6 POKEMON\n");
        printf("FOLLOWER: %s\n", "ACTIVE");
        printf("\n%s\n",message);

        frameCount++;
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();

    return 0;
}
