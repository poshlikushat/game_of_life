CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
LDFLAGS = -lncurses
SRC = game_of_life.c
TARGET = game_of_life

.PHONY: all clean cppcheck style

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem --std=c11 $(SRC)

style:
	clang-format --dry-run --Werror $(SRC)
