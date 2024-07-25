/**
  CHIP8
*/
#include <Keypad.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
extern "C"{
  #include "chip8/chip8.h"
};

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
  {'.', '0', '=', '/'}
};

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

/* TODO: Remove me in favor of some SD card reader or some more robust method */
void load_program(uint8_t *program)
{
  // Clear display
  program[0] = 0x00;
  program[1] = 0xE0;

  // Set V[1] to 0xA
  program[2] = 0x61;
  program[3] = 0x0A;

  // Set I to sprite
  program[4] = 0xF1;
  program[5] = 0x29;

  // Display (0, 0) height of 5
  program[6] = 0xD2;
  program[7] = 0x35;

  // Loop
  program[8] = 0x12;
  program[9] = 0x02;
}

void setup() {
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
  load_program(program_memory);

}

void loop() {
  Serial.print(cpu.state.memory[cpu.state.PC], HEX);
  Serial.print(cpu.state.memory[cpu.state.PC + 1], HEX);
  Serial.println();  
  chip8_run(&cpu);
  // char key = keypad.getKey();
  // if (key) {
  //   processInput(key);
  // }
}
