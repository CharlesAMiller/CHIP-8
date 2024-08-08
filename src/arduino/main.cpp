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

byte device_state = STATE_LAUNCHER;
byte selected_rom_idx = 0;
byte prev_selected_rom_idx = -1;

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

/**
 * Passed to our CHIP-8 instance as a display peripheral
 * This function takes the CHIP-8's screen buffer as a parameter and renders it pixel-by-pixel
 */
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

/**
 * Function that is called to draw the launcher menu - a list of selectable ROMs 
 */
void draw_launcher(Adafruit_ILI9341 *screen, unsigned int selected_index, const char *selections[], unsigned int length)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  int16_t x, y;
  uint16_t w, h;
  for (unsigned int i = 0; i < length; i++)
  {
    screen->setTextColor(ILI9341_WHITE);

    // Highlight the curently selected title
    if (i == selected_index)
    {
      screen->getTextBounds(selections[i], screen->getCursorX(), screen->getCursorY(), &x, &y, &w, &h);
      screen->fillRect(x, y, w, h, ILI9341_WHITE);
      screen->setTextColor(ILI9341_BLACK);
    }
    screen->println(selections[i]);
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
}

void loop()
{
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
      selected_rom_idx += (selected_rom_idx < ROMS_COUNT) ? 1 : 0;
      break;

    case '=':
      init_state(&(cpu.state), chip8_memory, program_memory);
      memcpy(program_memory, rom_programs[selected_rom_idx], rom_programs_sizes[selected_rom_idx]);
      device_state = STATE_RUNNING;
      break;

    case '/':
      device_state = (device_state == STATE_LAUNCHER) ? STATE_RUNNING : STATE_LAUNCHER;
      break;
    }
    if (prev_selected_rom_idx != selected_rom_idx)
      draw_launcher(&tft, selected_rom_idx, rom_names, ROMS_COUNT);
    prev_selected_rom_idx = selected_rom_idx;
  }
  else if (device_state == STATE_RUNNING)
  {
    Serial.print(cpu.state.memory[cpu.state.PC], HEX);
    Serial.print(cpu.state.memory[cpu.state.PC + 1], HEX);
    Serial.println();
    chip8_run(&cpu);
  }
}
