CC = clang
CFLAGS = -I/opt/homebrew/Cellar/csfml/2.6.1/include
LDFLAGS = -L/opt/homebrew/lib -lcsfml-graphics -lcsfml-window -lcsfml-system -lcsfml-audio

SRCS = main.c chip8.c audio.c io.c graphics.c
OBJS = $(SRCS:.c=.o)
TARGET = chip8 

# Variables for the test task
TEST_SRCS = test.c chip8.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_TARGET = test_chip8

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o $(TEST_TARGET) $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET)

.PHONY: clean
