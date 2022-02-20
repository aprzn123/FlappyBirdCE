#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include <debug.h>
#include <math.h>

#define MAX_PIPES 8

typedef struct Bird {
    int height;
    int vel;
} Bird;

typedef struct Pipe {
    int posX;
    int openingUpper;
    int openingLower;
} Pipe;

bool step(void);
void drawBird(Bird bird);
void drawPipes(Pipe** pipes);

void birdGravity(Bird* bird_p);
void flap(Bird* bird_p);
void movePipes(Pipe** pipes, int distance);
void addPipe(Pipe** pipes, char* allocatedPipes, int posX, int openingUpper, int openingLower);
void cleanPipes(Pipe** pipes, char* allocatedPipes);
bool intersect(Pipe** pipes, Bird* bird);

int main(void)
{
    bool partial_redraw = false;
    bool enter_pressed = false;
    bool enter_was_pressed = false;
    int frameCounter = 0;

    Bird bird;
    bird.height = 120;
    bird.vel = 0;

    Pipe** pipes = malloc(sizeof(Pipe*) * MAX_PIPES);
    for (int i = 0; i < MAX_PIPES; i++) *(pipes + i) = NULL;
    char allocatedPipes = 0x00;
    
    //Pipe test_pipe = {.posX = 80, .openingUpper = 70, .openingLower = 140};
    //*pipes = &test_pipe;

    srandom(rtc_Time());
    
    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_DOWN);
    timer_SetReload(1, 1000);

    /* Initialize graphics drawing */
    gfx_Begin();

    /* Draw to the buffer to avoid rendering artifacts */
    gfx_SetDrawBuffer();
 
    /* No rendering allowed in step! */
    while (step() && !intersect(pipes, &bird))
    {

        /* Only want to redraw part of the previous frame? */
        if (partial_redraw)
        {
            /* Copy previous frame as a base for this frame */
            gfx_BlitScreen();
        }

        enter_pressed = kb_IsDown(kb_KeyEnter);
        birdGravity(&bird);
        if (enter_pressed && !enter_was_pressed) flap(&bird);
        movePipes(pipes, 2);
        if (frameCounter % 30 == 0) 
        {
            int pos = randInt(0, 240-70);
            addPipe(pipes, &allocatedPipes, 360, pos, pos + 70);
        }
        if (frameCounter % 5 == 0) cleanPipes(pipes, &allocatedPipes);

        gfx_FillScreen(0xFF);
        drawPipes(pipes);
        drawBird(bird);

        /* Queue the buffered frame to be displayed */
        gfx_SwapDraw();

        enter_was_pressed = enter_pressed;
        frameCounter++;
        usleep(31.25 * timer_Get(1));
    }

    /* End graphics drawing */
    gfx_End();

    for (int i = 0; i < 8; i++)
    {
        if ((allocatedPipes & (1 << i)) != 0) free(*(pipes + i));
    }
    free(pipes);

    return 0;
}

/* Implement me! */
bool step(void)
{
    kb_Scan();
    return !kb_IsDown(kb_KeyMode);
}

void drawBird(Bird bird)
{
    gfx_SetColor(0xE0);
    gfx_FillCircle(20, bird.height, 10);
}

void drawPipes(Pipe** pipes)
{
    gfx_SetColor(0x03);
    for (int i = 0; i < MAX_PIPES; i++)
    {
        Pipe* pipe = *(pipes + i);
        if (!pipe) continue;
        gfx_VertLine(pipe->posX, 0, pipe->openingUpper);
        gfx_VertLine(pipe->posX, pipe->openingLower, 240 - pipe->openingLower);
    }
}

void birdGravity(Bird* bird_p)
{
    bird_p->vel ++;
    bird_p->height += bird_p->vel;
}

void flap(Bird* bird_p)
{
    bird_p->vel = -10;
}

void movePipes(Pipe** pipes, int distance)
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        Pipe* pipe_p = *(pipes + i);
        if (pipe_p == NULL) continue;
        pipe_p->posX -= distance;
    }
}

void addPipe(Pipe** pipes, char* allocatedPipes, int posX, int openingUpper, int openingLower)
{
    for (int i = 0; i < 8; i++)
    {
        char mask = 1 << i;
        if ((*allocatedPipes & mask) == 0x00)
        {
            Pipe* newPipe = malloc(sizeof(Pipe));
            newPipe->posX = posX;
            newPipe->openingUpper = openingUpper;
            newPipe->openingLower = openingLower;
            *(pipes + i) = newPipe;
            *allocatedPipes |= mask;
            return;
        }
    }
}

void cleanPipes(Pipe** pipes, char* allocatedPipes)
{
    for (int i = 0; i < 8; i++) {
        if ((*allocatedPipes & 1 << i) == 0 || (*(pipes + i))->posX > 0) continue;
        *allocatedPipes &= ~(1 << i);
        free(*(pipes + i));
        *(pipes + i) = NULL;
    }
}

bool intersect(Pipe** pipes, Bird* bird)
{
    for (int i = 0; i < 8; i++)
    {
        Pipe* pipe_p = *(pipes + i);
        if (pipe_p != NULL && 10 <= pipe_p->posX && 30 >= pipe_p->posX)
        {
            int upper = pipe_p->openingUpper - bird->height;
            int lower = pipe_p->openingLower - bird->height;
            int x = pipe_p->posX - 20;
            int y = sqrt(100 - x * x);
            dbg_sprintf(dbgout, "up: %d, dn: %d, y: %d\n", upper, lower, y);
            if ((double) upper > -y || (double) lower < y) return true;
        }
    }
    return false;
}