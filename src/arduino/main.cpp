/**
  CHIP8
*/
#include <Keypad.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
extern "C"
{
#include "chip8/chip8.h"
};
#include "roms/roms.h"

/* Device State */
#define STATE_LAUNCHER 0
#define STATE_RUNNING 1

byte device_state = STATE_RUNNING;
byte selected_rom_idx = 2;

/* Display */
#define TFT_DC 28
#define TFT_CS 30
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/* CHIP-8 */
chip8 cpu;
uint8_t chip8_memory[RAM_SIZE];
uint8_t *program_memory = &chip8_memory[PROGRAM_OFFSET];

/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', '+'},
    {'4', '5', '6', '-'},
    {'7', '8', '9', '*'},
    {'.', '0', '=', '/'}};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

void draw(uint8_t *screen_buffer)
{
  uint8_t x, y, pixels;
  x = 0;
  y = 0;
  uint16_t color;
  for (int i = 0; i < SCREEN_BYTES; i++)
  {
    if (i % H_OFFSET == 0)
    {
      x = 0;
      y++;
    }

    pixels = screen_buffer[i];

    for (int j = 8; j > 0; j--, pixels <<= 1)
    {
      color = ((pixels & 0b10000000) != 0) ? ILI9341_RED : ILI9341_WHITE;
      tft.drawPixel(x++, y, color);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  tft.begin();

  peripherals peripherals;
  peripherals.display = &draw;
  peripherals.get_key_pressed = NULL;
  peripherals.is_key_pressed = NULL;
  peripherals.random = NULL;
  peripherals.noise = NULL;

  chip8_config config = {&peripherals, chip8_memory, program_memory};

  cpu = chip8_init(&config);
  memcpy(program_memory, rom_programs[selected_rom_idx], rom_programs_sizes[selected_rom_idx]);
}

void loop()
{
  Serial.print(cpu.state.memory[cpu.state.PC], HEX);
  Serial.print(cpu.state.memory[cpu.state.PC + 1], HEX);
  Serial.println();
  if (device_state == STATE_LAUNCHER)
  {
    char key = keypad.getKey();
    switch (key) 
    {
      // Up on d-pad
      case '2':
        selected_rom_idx -= (selected_rom_idx > 0) ? 1 : 0;
        break;

      // Down on d-pad 
      case '8':
        selected_rom_idx += (selected_rom_idx < 16) ? 1 : 0;
        break;
      
      case '=':
        device_state = (device_state == STATE_LAUNCHER) ? STATE_RUNNING : STATE_LAUNCHER;
        break;
    }
  }
  else if (device_state == STATE_RUNNING)
  {
    chip8_run(&cpu);
  }
}
