#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

/* Display */

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA 12
#define SCL 11
// I2C display address
#define OLED_ADDR 0x3C

/**
  CHIP8
*/

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

/* CHIP-8 */
chip8 cpu;
uint8_t chip8_memory[RAM_SIZE];
peripherals periphs;

/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {42, 41, 40, 39};
byte colPins[KEYPAD_COLS] = {38, 37, 36, 35};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', '+'},
    {'4', '5', '6', '-'},
    {'7', '8', '9', '*'},
    {'.', '0', '=', '/'}};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
char last_pressed_key;
KeyState key_state;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
      y += 2;
    }

    pixels = screen_buffer[i];

    for (int j = 8; j > 0; j--, pixels <<= 1)
    {
      color = ((pixels & 0b10000000) != 0) ? SSD1306_WHITE : SSD1306_BLACK;
      display.drawRect(x, y, 2, 2, color);
      x += 2;
    }
  }

  display.display();
}

/**
 * Function that is called to draw the launcher menu - a list of selectable ROMs
 */
void draw_launcher(Adafruit_SSD1306 *screen, unsigned int selected_index, const char *selections[], unsigned int length)
{
  Serial.println("Drawing Launcher");
  display.clearDisplay();
  display.setCursor(0, 0);
  int16_t x, y;
  uint16_t w, h;
  for (unsigned int i = 0; i < length; i++)
  {
    screen->setTextColor(SSD1306_WHITE);

    // Highlight the curently selected title
    if (i == selected_index)
    {
      screen->getTextBounds(selections[i], screen->getCursorX(), screen->getCursorY(), &x, &y, &w, &h);
      screen->fillRect(x, y, w, h, SSD1306_WHITE);
      screen->setTextColor(SSD1306_BLACK);
    }
    screen->println(selections[i]);
  }
  display.display();
}

uint8_t get_key()
{
  char key = keypad.waitForKey();

  switch (key)
  {
    case '.':
      return 0xA;
    case '=':
      return 0xB;
    case '+':
      return 0xC;
    case '-':
      return 0xD; 
    case '*':
      return 0xE;
    case '/':
      return 0xF;
    default:
      return key - '0';
  }
  return key;
}

uint8_t is_key_pressed(uint8_t key)
{
  char key_char;

  switch (key)
  {
    case 0xA:
      key_char = '.';
      break;
    case 0xB:
      key_char = '=';
      break;
    case 0xC:
      key_char = '+';
      break;
    case 0xD:
      key_char = '-';
      break;
    case 0xE:
      key_char = '*';
      break;
    case 0xF:
      key_char = '/';
      break;
    default:
      key_char = key + '0';
      break;
  }

  return key_char == last_pressed_key && (key_state == HOLD || key_state == PRESSED);
}

uint8_t random_byte()
{
  return random(0, 255);
}

void noise()
{
}

void setup()
{
  Serial.begin(115200);
  delay(2000); // Waiting for Serial.

  Serial.println("Running setup");

  Wire.begin(SDA, SCL);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Loop forever if there's an error
  }

  // Clear the buffer
  display.clearDisplay();

  // Display some initial text
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(F("Hello, CHIP8!"));

  // Update the display with the written text
  display.display();
  // delay(2000);

  Serial.println("Adding peripherals");

  periphs.display = &draw;
  periphs.get_key_pressed = &get_key;
  periphs.is_key_pressed = &is_key_pressed;
  periphs.random = &random_byte;
  periphs.noise = &noise;

  keypad.setDebounceTime(5);

  chip8_config config = {&periphs, chip8_memory, NULL};

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
      init_state(&(cpu.state), chip8_memory);
      chip8_load_program(&cpu, ((uint8_t *)rom_programs[selected_rom_idx]), rom_programs_sizes[selected_rom_idx]);
      device_state = STATE_RUNNING;
      Serial.println("Starting ROM");
      display.clearDisplay();
      display.display();
      break;

    case '/':
      device_state = (device_state == STATE_LAUNCHER) ? STATE_RUNNING : STATE_LAUNCHER;
      break;
    }
    if (prev_selected_rom_idx != selected_rom_idx)
      draw_launcher(&display, selected_rom_idx, rom_names, ROMS_COUNT);
    prev_selected_rom_idx = selected_rom_idx;
  }
  else if (device_state == STATE_RUNNING)
  {
    key_state = keypad.getState();
    char key = keypad.getKey();
    if (key != NO_KEY)
      last_pressed_key = key;
    // Debugging instructions
    // Serial.print(cpu.state.memory[cpu.state.PC], HEX);
    // Serial.print(cpu.state.memory[cpu.state.PC + 1], HEX);
    // Serial.println();
    chip8_run(&cpu);
  }
}
