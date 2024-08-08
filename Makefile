CC = clang
CFLAGS = -I/opt/homebrew/Cellar/csfml/2.6.1/include
LDFLAGS = -L/opt/homebrew/lib -lcsfml-graphics -lcsfml-window -lcsfml-system -lcsfml-audio

APP_SRCS = src/app/main.c src/chip8/chip8.c src/app/audio.c src/app/io.c src/app/graphics.c
OBJS = $(APP_SRCS:.c=.o)
TARGET = chip8 

# Variables for the test task
TEST_SRCS = src/test/test.c src/chip8/chip8.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_TARGET = test_chip8

# Building roms
BUILD_ROMS_TARGET = build_roms
ROMS_DIR := ./roms/games
OUTPUT_DIR := ./src/arduino/roms
# Find all ROM files in the ROMS_DIR
ROM_FILES := $(wildcard $(ROMS_DIR)/*)
# Generate output header file paths
ROM_HEADER_FILES := $(patsubst $(ROMS_DIR)/%, $(OUTPUT_DIR)/%.h, $(ROM_FILES))
INDEX_FILE := $(OUTPUT_DIR)/roms.h

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o $(TEST_TARGET) $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_ROMS_TARGET): $(ROM_HEADER_FILES) $(INDEX_FILE)

# Rule to convert each ROM file to a header file
$(OUTPUT_DIR)/%.h: $(ROMS_DIR)/%
	mkdir -p $(OUTPUT_DIR)
	xxd -i $< > $@
	@echo "const char* $(notdir $*) = \"$(notdir $*)\";" >> $@

# Rule to generate index header file
$(INDEX_FILE): $(ROM_HEADER_FILES)
	@echo "// ROM Index Header File" >> $(INDEX_FILE)
	@for file in $(ROM_HEADER_FILES); do \
		name=$$(basename $$file .h); \
		name=$$(echo $$name | sed 's/\./_/g'); \
		echo "#include \"$$name.h\"" >> $(INDEX_FILE); \
	done
	@echo >> $(INDEX_FILE)
	@echo "#define ROMS_COUNT $(words $(strip $(ROM_HEADER_FILES)))" >> $(INDEX_FILE)
	@echo >> $(INDEX_FILE)
	@echo "const char* rom_names[] = {" >> $(INDEX_FILE)
	@for file in $(ROM_HEADER_FILES); do \
		name=$$(basename $$file .h); \
		name=$$(echo $$name | sed 's/\./_/g'); \
		echo "    \"$$name\"," >> $(INDEX_FILE); \
	done
	@echo "};" >> $(INDEX_FILE)
	@echo >> $(INDEX_FILE)
	@echo "const unsigned char *rom_programs[] = {" >> $(INDEX_FILE)
	@for file in $(ROM_HEADER_FILES); do \
		name=$$(basename $$file .h); \
		name=$$(echo $$name | sed 's/\./_/g'); \
		echo "    roms_games_$$name," >> $(INDEX_FILE); \
	done
	@echo "};" >> $(INDEX_FILE)
	@echo >> $(INDEX_FILE)
	@echo "const unsigned int rom_programs_sizes[] = {" >> $(INDEX_FILE)
	@for file in $(ROM_HEADER_FILES); do \
		name=$$(basename $$file .h); \
		name=$$(echo $$name | sed 's/\./_/g'); \
		echo "    roms_games_$${name}_len," >> $(INDEX_FILE); \
	done
	@echo "};" >> $(INDEX_FILE)
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET) $(ROM_HEADER_FILES) $(INDEX_FILE)

.PHONY: clean
