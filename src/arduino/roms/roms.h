// ROM Index Header File
#include "br8kout.h"
#include "down8.h"
#include "test.h"
#include "tetris.h"

#define ROMS_COUNT 4

const char* rom_names[] = {
    "br8kout",
    "down8",
    "test",
    "tetris",
};

const unsigned char *rom_programs[] = {
    roms_games_br8kout,
    roms_games_down8,
    roms_games_test,
    roms_games_tetris,
};

const unsigned int rom_programs_sizes[] = {
    roms_games_br8kout_len,
    roms_games_down8_len,
    roms_games_test_len,
    roms_games_tetris_len,
};
