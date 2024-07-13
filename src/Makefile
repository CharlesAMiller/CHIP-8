CC = clang
CFLAGS = -I/opt/homebrew/Cellar/csfml/2.6.1/include
LDFLAGS = -L/opt/homebrew/lib -lcsfml-graphics -lcsfml-window -lcsfml-system -lcsfml-audio

SRCS = main.c chip8.c
OBJS = $(SRCS:.c=.o)
TARGET = chip8 

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)