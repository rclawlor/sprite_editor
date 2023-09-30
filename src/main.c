#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "palette.h"

// Constants
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 360;

const int BUTTON_SIZE = 16;
const int COLOURS = 64;
const int COLUMNS = 4;

const int TILE_SIZE = 8;
const int N_TILES = 128;
const int SPRITE_COLUMNS = 8;
const int RENDER_SIZE = 24;

const int SPRITE_SHEET_X = 50;
const int SPRITE_SHEET_Y = 50;

// Data types
typedef struct {
    int input[256];
} KeyInput;

typedef struct {
    int colour;
    int colour_index;
    int x;
    int y;
    bool selected;
} ColourButton;

typedef struct {
    bool click;
    int x;
    int y;
} MouseInput;

typedef struct {
    int colour_data[64];
    int index;
    int x;
    int y;
    bool selected;
} Sprite;

// Definitions
void draw_colour_button(int *pixels, ColourButton *button);
void colour_select(ColourButton *button, MouseInput *mouse, ColourButton *current_colour);
void draw_pixel(int *pixels, int pixel_x, int pixel_y, ColourButton *colour, Sprite *sprite, Sprite sprite_sheet[]);
void draw_sprite(int *pixels, Sprite *sprite, int scale);
void draw_sprite_sheet(int *pixels, int x, int y, Sprite sprite_sheet[]);
void sprite_select(MouseInput *mouse, Sprite *sprite, Sprite *current_sprite);
void draw_canvas(int *pixels, Sprite *sprite, Sprite sprite_sheet[]);

// Main loop
int main()
{
    // Check if SDL has intialised
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
        return 1;
    }
    // Define window
    SDL_Window *window = SDL_CreateWindow("Sprite Editor",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_RESIZABLE);
    // Return error if window not initialised
    if (!window)
    {
        fprintf(stderr, "Could not create window\n");
        return 1;
    }
    // Define software renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    // Return error if renderer not initialised
    if (!renderer)
    {
        fprintf(stderr, "Could not create renderer\n");
        return 1;
    }

    // Limit minimum window size to dimensions of screen buffer
    SDL_SetWindowMinimumSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderSetIntegerScale(renderer, 1);
    
    SDL_Texture *screen_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);

    unsigned int *pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);

    // Game variables
    int desired_fps = 60; 
    int last_ticks = SDL_GetTicks();

    // Rendering loop
    SDL_Event event;

    KeyInput keyboard = {
        .input = {}
    };

    MouseInput mouse = {
        .click = false,
        .x = 0,
        .y = 0
    };

    // Set every pixel to white.
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            pixels[x + y * SCREEN_WIDTH] = 0x232324ff;
        }
    }

    // Draw working grid
    for (int i=SCREEN_WIDTH/2 - RENDER_SIZE*TILE_SIZE/2; i<=SCREEN_WIDTH/2 + RENDER_SIZE*TILE_SIZE/2; ++i)
    {
        for (int j=SCREEN_HEIGHT/2 - RENDER_SIZE*TILE_SIZE/2; j<=SCREEN_HEIGHT/2 + RENDER_SIZE*TILE_SIZE/2; ++j)
        {
            if ((i-SCREEN_WIDTH/2)%RENDER_SIZE==0 || (j-SCREEN_HEIGHT/2)%RENDER_SIZE==0)
            {
                pixels[i + j*SCREEN_WIDTH] = 0x000000ff;
            }
        }
    }

    ColourButton colour_palette[COLOURS];
    int colour_index = 0;
    int palette_x = 512;
    int palette_y = 64;
    // Create array of colour buttons
    for (int i=0; i<COLOURS/COLUMNS; ++i)
    {
        for (int j=0; j<COLUMNS; ++j)
        {
            colour_index = i + j*COLOURS/COLUMNS;
            colour_palette[colour_index].colour = palette[colour_index];
            colour_palette[colour_index].colour_index = colour_index;
            colour_palette[colour_index].x = palette_x + j*BUTTON_SIZE;
            colour_palette[colour_index].y = palette_y + i*BUTTON_SIZE;
            colour_palette[colour_index].selected = false;
        }
    }

    ColourButton current_colour = {
        .colour = 0x00000000,
        .colour_index = 0,
        .x = 0,
        .y = 0,
    };

    FILE *sprite;
    Sprite sprite_sheet[N_TILES];

    for (int i=0; i<N_TILES; ++i)
    {
        sprite_sheet[i].index = i;
        sprite_sheet[i].selected = false;
        sprite_sheet[i].x = 2*(i%TILE_SIZE)*TILE_SIZE + SPRITE_SHEET_X;
        sprite_sheet[i].y = 2*(i/TILE_SIZE)*TILE_SIZE + SPRITE_SHEET_Y;
        
        for (int j=0; j<TILE_SIZE*TILE_SIZE; ++j)
        {
            sprite_sheet[i].colour_data[j] = 0;
        }
    }

    Sprite current_sprite = sprite_sheet[0];

    while (1)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                exit(0);
            }
            // Check for keypresses
            if (event.type == SDL_KEYDOWN) {
                keyboard.input[event.key.keysym.scancode] = true;
            }
            // Check for key releases
            if (event.type == SDL_KEYUP) {
                keyboard.input[event.key.keysym.scancode] = false;
            }
            // Check for mouse presses
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                mouse.click = true;
                mouse.x = event.button.x;
                mouse.y = event.button.y;
            }
            // Check for mouse motion
            if (event.type == SDL_MOUSEMOTION) {
                mouse.x = event.motion.x;
                mouse.y = event.motion.y;
            } 
            // Check for mouse releases
            if (event.type == SDL_MOUSEBUTTONUP) {
                mouse.click = false;
                mouse.x = event.button.x;
                mouse.y = event.button.y;
            }
        }

        // Fix framerate
        if (SDL_GetTicks() - last_ticks < 1000/desired_fps)
        {
            continue;
        }

        // Process input
        if (mouse.click)
        {
            // Check what colour was selected
            for (int i=0; i<COLOURS; ++i)
            {
                colour_select(&colour_palette[i], &mouse, &current_colour);
            }
            colour_palette[current_colour.colour_index].selected = true;
            for (int i=0; i<N_TILES; ++i)
            {
                sprite_select(&mouse, &sprite_sheet[i], &current_sprite);
            }
            sprite_sheet[current_sprite.index].selected = true;

            // Draw current pixel
            int pixel_x = (mouse.x - (SCREEN_WIDTH/2 - RENDER_SIZE*TILE_SIZE/2))/RENDER_SIZE;
            int pixel_y = (mouse.y - (SCREEN_HEIGHT/2 - RENDER_SIZE*TILE_SIZE/2))/RENDER_SIZE;

            if (pixel_x>=0 && pixel_x<TILE_SIZE && pixel_y>=0 && pixel_y<TILE_SIZE)
            {
                // draw_pixel(pixels, pixel_x, pixel_y, &current_colour, sprite_data);
                draw_pixel(pixels, pixel_x, pixel_y, &current_colour, &current_sprite, sprite_sheet);
            }
        }

        for (int i=0; i<COLOURS; ++i)
        {
            draw_colour_button(pixels, &colour_palette[i]);
        }

        draw_canvas(pixels, &current_sprite, sprite_sheet);
        draw_sprite_sheet(pixels, 20, 20, sprite_sheet);

        // if (keyboard.input[SDL_SCANCODE_S])
        // {
        //     sprite = fopen("../sprite.c", "w");
        //     fprintf(sprite, "int sprite[] = {\n");
        //     for (int i=0; i<TILE_SIZE*TILE_SIZE; ++i)
        //     {
        //         fprintf(sprite, "\t%d,\n", sprite_data[i]);
        //     }
        //     fprintf(sprite, "};\n");
        //     fclose(sprite);
        // }

        // It's a good idea to clear the screen every frame,
        // as artifacts may occur if the window overlaps with
        // other windows or transparent overlays.
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        last_ticks = SDL_GetTicks();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void draw_colour_button(int *pixels, ColourButton *button)
{
    int button_index = button->x + (button->y)*SCREEN_WIDTH;
    if (button->selected)
    {
        for (int i=0; i<BUTTON_SIZE; ++i) {
            for (int j=0; j<BUTTON_SIZE; ++j) {
                if (i==0 || i==BUTTON_SIZE-1 || j==0 || j==BUTTON_SIZE-1)
                {
                    pixels[button_index + i + j*SCREEN_WIDTH] = 0xffffff00;
                }
                else
                {
                    pixels[button_index + i + j*SCREEN_WIDTH] = button->colour;
                }
            }
        }
    }
    else
    {
        for (int i=0; i<BUTTON_SIZE; ++i) {
            for (int j=0; j<BUTTON_SIZE; ++j) {
                pixels[button_index + i + j*SCREEN_WIDTH] = button->colour;
            }
        }
    }
};

void colour_select(ColourButton *button, MouseInput *mouse, ColourButton *current_colour)
{
    if (0 <= mouse->x - button->x &&
        mouse->x - button->x <BUTTON_SIZE &&
        0 <= mouse->y - button->y &&
        mouse->y - button->y <BUTTON_SIZE)
    {
        current_colour->colour = button->colour;
        current_colour->colour_index = button-> colour_index;
    }
    button->selected = false;
};

void draw_pixel(int *pixels, int pixel_x, int pixel_y, ColourButton *colour, Sprite *sprite, Sprite sprite_sheet[])
{
    int area_corner_x = SCREEN_WIDTH/2 - RENDER_SIZE*TILE_SIZE/2;
    int area_corner_y = SCREEN_HEIGHT/2 - RENDER_SIZE*TILE_SIZE/2;

    int buffer_index = area_corner_x + area_corner_y*SCREEN_WIDTH + pixel_x*RENDER_SIZE + pixel_y*RENDER_SIZE*SCREEN_WIDTH;

    // for (int i=1; i<RENDER_SIZE; ++i) {
    //     for (int j=1; j<RENDER_SIZE; ++j) {
    //         pixels[buffer_index + i + j*SCREEN_WIDTH] = palette[colour->colour_index];
    //     }
    // }

    sprite_sheet[sprite->index].colour_data[pixel_x + TILE_SIZE*pixel_y] = colour->colour_index;
};

void draw_canvas(int *pixels, Sprite *sprite, Sprite sprite_sheet[])
{
    int area_corner_x = SCREEN_WIDTH/2 - RENDER_SIZE*TILE_SIZE/2;
    int area_corner_y = SCREEN_HEIGHT/2 - RENDER_SIZE*TILE_SIZE/2;

    int buffer_index = area_corner_x + area_corner_y*SCREEN_WIDTH; //+ pixel_x*RENDER_SIZE + pixel_y*RENDER_SIZE*SCREEN_WIDTH;

    for (int i=0; i<RENDER_SIZE*TILE_SIZE; ++i) {
        for (int j=0; j<RENDER_SIZE*TILE_SIZE; ++j) {
            pixels[buffer_index + i + j*SCREEN_WIDTH] = palette[
                sprite_sheet[sprite->index].colour_data[i/RENDER_SIZE + (j/RENDER_SIZE)*TILE_SIZE]];
        }
    }
}

// Add scale input to draw_sprite() so it can be reused for main window
void draw_sprite(int *pixels, Sprite *sprite, int scale)
{
    int buffer_index = sprite->x + sprite->y*SCREEN_WIDTH;

    if (sprite->selected)
    {
        for (int i=0; i<scale*TILE_SIZE; ++i)
        {
            for (int j=0; j<scale*TILE_SIZE; ++j)
            {
                if (i==0 || i==scale*TILE_SIZE-1 || j==0 || j==scale*TILE_SIZE-1)
                {
                    pixels[buffer_index + i + j*SCREEN_WIDTH] = 0xffffff00;
                }
                else
                {
                pixels[buffer_index + i + j*SCREEN_WIDTH] = palette[
                    sprite->colour_data[i/scale + (j/scale)*TILE_SIZE]
                    ];
                }
            }
        }
    }
    else
    {
        for (int i=0; i<2*TILE_SIZE; ++i)
        {
            for (int j=0; j<2*TILE_SIZE; ++j)
            {
                pixels[buffer_index + i + j*SCREEN_WIDTH] = palette[
                    sprite->colour_data[i/2 + (j/2)*TILE_SIZE]
                    ];
            }
        }
    }
}

void draw_sprite_sheet(int *pixels, int x, int y, Sprite sprite_sheet[])
{
    for (int i=0; i<N_TILES; ++i)
    {
        draw_sprite(pixels, &sprite_sheet[i], 2);
    }
}

void sprite_select(MouseInput *mouse, Sprite *sprite, Sprite *current_sprite)
{
    if (sprite->x <= mouse->x &&
        mouse->x < sprite->x+2*TILE_SIZE &&
        sprite->y <= mouse->y &&
        mouse->y < sprite->y+2*TILE_SIZE)
    {
        current_sprite->index = sprite->index;
    }
    sprite->selected = false;
}