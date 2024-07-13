/** @file main.c
 * TODO:
 *  - Finish adding all op codes
 *  - Finish adding execution behavior
 *  - (Done) Separate into different files
 *  - Test that the code compiles to Arduino
 *  - Add functionality tests
 */

#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

void noise();
u_int8_t get_key_pressed();
u_int8_t is_key_pressed(u_int8_t key);
u_int8_t rand_byte();

void load_program(char *file_name, u_int8_t *program);
void draw_screen(u_int8_t *screen);
void generateSineWave(int16_t *samples, size_t sampleCount, float frequency, float amplitude, unsigned sampleRate);
sfSoundBuffer *createSineWaveBuffer(float frequency, float amplitude, unsigned sampleRate, float duration);

sfImage *image;
sfTexture *texture;
sfSprite *sprite;
sfWindow *window;

sfKeyCode keyMap[16] = {
    [0] = sfKeyNum0,
    [1] = sfKeyNum1,
    [2] = sfKeyNum2,
    [3] = sfKeyNum3,
    [4] = sfKeyNum4,
    [5] = sfKeyNum5,
    [6] = sfKeyNum6,
    [7] = sfKeyNum7,
    [8] = sfKeyNum8,
    [9] = sfKeyNum9,
    [0xA] = sfKeyA,
    [0xB] = sfKeyB,
    [0xC] = sfKeyC,
    [0xD] = sfKeyD,
    [0xE] = sfKeyE,
    [0xF] = sfKeyF};

int main(int argc, char *argv[])
{

    srand(time(0));

    // Setup
    peripherals peripherals;
    peripherals.display = &draw_screen;
    peripherals.get_key_pressed = &get_key_pressed;
    peripherals.is_key_pressed = &is_key_pressed;
    peripherals.random = &rand_byte;
    peripherals.noise = &noise;

    u_int8_t memory[RAM_SIZE];
    u_int8_t program_memory[PROGRAM_SIZE];
    load_program(argv[1], program_memory);

    chip8_config config;
    config.peripherals = &peripherals;
    config.memory = memory;
    config.program = program_memory;

    chip8 cpu = init(&config);

    const sfVideoMode mode = {SCREEN_W * 16, SCREEN_H * 16, 32};
    window = sfRenderWindow_create(mode, "CHIP-8", sfResize | sfClose, NULL);

    image = sfImage_create(SCREEN_W, SCREEN_H);
    sfImage_createMaskFromColor(image, sfBlack, 0);

    if (!window)
        return EXIT_FAILURE;

    texture = sfTexture_createFromImage(image, NULL);
    sprite = sfSprite_create();
    
    sfSprite_setTexture(sprite, texture, sfTrue);
    sfVector2f scale;
    scale.x = 8;
    scale.y = 8;
    sfSprite_setScale(sprite, scale);

    sfEvent event;
    while (sfRenderWindow_isOpen(window))
    {
        run(&cpu);

        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed)
                sfRenderWindow_close(window);
        }

        // sfRenderWindow_clear(window, sfBlack);
        sfTime sleepTime = { 500 };
        sfSleep(sleepTime);
    }
}

void draw_screen(u_int8_t *screen)
{
    u_int8_t x = 0;
    u_int8_t y = 0;
    u_int8_t pixels;
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
            sfImage_setPixel(image, x, y, color);
            color = ((pixels & 0b10000000) != 0) ? sfWhite : sfBlack;
            x++;
        }
    }

    sfTexture_updateFromImage(texture, image, 0, 0);
    sfRenderWindow_drawSprite(window, sprite, NULL);
    sfRenderWindow_display(window);

}

// Still a placeholder
void load_program(char* file_name, u_int8_t *program)
{
    long lSize;
    FILE* file = fopen(file_name, "rb");
        // obtain file size:
    fseek (file, 0 , SEEK_END);
    lSize = ftell (file);
    rewind (file);

    fread(program, 1, lSize, file);
}

u_int8_t rand_byte()
{
    u_int8_t r = rand() % 256;
    return r;
}

u_int8_t get_key_pressed()
{
    // Block
    while (1)
    {
        for (int i = 0; i < 0xF; i++)
            if (is_key_pressed(i))
                return i;
    }
}

u_int8_t is_key_pressed(u_int8_t key)
{
    sfBool isPresed = sfKeyboard_isKeyPressed(keyMap[key]);
    return (isPresed == sfTrue) ? 1 : 0;
}

void generateSineWave(int16_t *samples, size_t sampleCount, float frequency, float amplitude, unsigned sampleRate)
{
    const float PI = 3.14159265358979323846f;
    for (size_t i = 0; i < sampleCount; i++)
    {
        float t = (float)i / (float)sampleRate;
        samples[i] = (int16_t)(amplitude * 32767 * sin(2 * PI * frequency * t));
    }
}

sfSoundBuffer *createSineWaveBuffer(float frequency, float amplitude, unsigned sampleRate, float duration)
{
    size_t sampleCount = (size_t)(sampleRate * duration);
    int16_t *samples = malloc(sampleCount * sizeof(int16_t));

    generateSineWave(samples, sampleCount, frequency, amplitude, sampleRate);

    sfSoundBuffer *buffer = sfSoundBuffer_createFromSamples(samples, sampleCount, 1, sampleRate);
    free(samples);

    return buffer;
}

void noise()
{
    float frequency = 440.0f;    // A4 note frequency
    float amplitude = 0.5f;      // Amplitude (0.0 to 1.0)
    unsigned sampleRate = 44100; // Standard sample rate
    float duration = .1f;       // Duration in seconds

    sfSoundBuffer *buffer = createSineWaveBuffer(frequency, amplitude, sampleRate, duration);
    if (!buffer)
    {
        return; // Error handling
    }

    sfSound *sound = sfSound_create();
    sfSound_setBuffer(sound, buffer);
    sfSound_play(sound);

    // Keep the application running while the sound plays
    sfTime durationTime = sfSeconds(duration);
    sfSleep(durationTime);

    // Cleanup
    sfSound_destroy(sound);
    sfSoundBuffer_destroy(buffer);
}