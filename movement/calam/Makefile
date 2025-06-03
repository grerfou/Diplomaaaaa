CC = clang
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo

SRC_DIR = src
OBJ_DIR = obj
BIN = cuneiform_sim

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

$(BIN): $(OBJECTS)
	@mkdir -p bin
	$(CC) $(OBJECTS) -o bin/$(BIN) $(LDFLAGS)
	@echo "✅ Build completed: ./bin/$(BIN)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run

run: $(BIN)
	./bin/$(BIN)

clean:
	rm -rf $(OBJ_DIR) bin
	@echo "🧹 Cleaned!"

