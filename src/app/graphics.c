/**
 * @file graphics.c 
 * @brief Implementation of graphics features
 */
#include "graphics.h"

sfImage *image = NULL; 
sfTexture *texture = NULL;
sfSprite *sprite = NULL;
sfWindow *window = NULL;

int init_screen(int width, int height, float scale_factor)
{
    const sfVideoMode mode = {width, height, 32};
    window = sfRenderWindow_create(mode, "CHIP-8", sfResize | sfClose, NULL);

    image = sfImage_create(SCREEN_W, SCREEN_H);
    sfImage_createMaskFromColor(image, sfBlack, 0);

    if (!window)
        return EXIT_FAILURE;

    texture = sfTexture_createFromImage(image, NULL);
    sprite = sfSprite_create();
    
    sfSprite_setTexture(sprite, texture, sfTrue);
    sfVector2f scale;
    scale.x = scale_factor;
    scale.y = scale_factor;
    sfSprite_setScale(sprite, scale);

    return 0;
}

void draw_screen(uint8_t *screen)
{
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t pixels;
    sfColor color;

    for (int i = 0; i < SCREEN_BYTES; i++)
    {

        if (i % H_OFFSET == 0)
        {
            x = 0;
            y++;
        }

        pixels = screen[i];
        color = sfBlack;
        for (int j = 8; j > 0; j--, pixels <<= 1)
        {
            color = ((pixels & 0b10000000) != 0) ? sfWhite : sfBlack;
            sfImage_setPixel(image, x++, y, color);
        }
    }

    sfTexture_updateFromImage(texture, image, 0, 0);
    sfRenderWindow_drawSprite(window, sprite, NULL);
    sfRenderWindow_display(window);
}

void start_render_loop(chip8 *cpu, unsigned int frame_limit)
{
    // Kind of hacky solution to the CHIP-8 timing
    sfWindow_setFramerateLimit(window, frame_limit);

    sfEvent event;
    while (sfRenderWindow_isOpen(window))
    {
        chip8_run(cpu);

        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed)
                sfRenderWindow_close(window);
        }

    }
}