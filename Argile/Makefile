CC = gcc
#CFLAGS = -Wall -Wextra -std=c99 -O2 -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/
CFLAGS = -Wall -Wextra -std=c99 -O2 -I raylib/include -L raylib/lib
#LDFLAGS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
LDFLAGS = -lraylib -lm

SRC_DIR = src
OBJ_DIR = obj
BIN = titan_sim

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN)


