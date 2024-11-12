EXECUTABLE= chip-8
SOURCE_FILES= chip8.c main.c
SOURCE_FILES_PATH= $(addprefix src/,$(SOURCE_FILES))
TESTS= setup fetch decode execute cycle
TEST_TARGETS= $(addprefix test-,$(TESTS))
SERVER_PORT= 8080

.PHONY: help
help:
	@echo "CHIP-8 emulator project"
	@echo "^^^^^^^^^^^^^^^^^^^^^^^"
	@echo "  help          show this message"
	@echo "  clean         cleaning up 'bin' folder"
	@echo "  all           building for all targets"
	@echo "  linux         building project for Linux"
	@echo "  windows       building project for Windows"
	@echo "  web           building project for web"
	@echo "  serve         serving web project on port $(SERVER_PORT)"
	@echo "  test-all      run all tests"
	@echo "  test-<file>   build and run test from 'test' folder"

.PHONY: clean
clean:
	@echo "Cleaning up 'bin' directory:"
	rm -rf ./bin

.PHONY: all
all: linux windows web

.PHONY: linux
linux: bin/linux
	@echo "Building for Linux:"
	gcc -o $</$(EXECUTABLE) $(SOURCE_FILES_PATH) `pkg-config --cflags --libs sdl2`

.PHONY: windows
windows: bin/windows
	@echo "Building for Windows:"
	x86_64-w64-mingw32-gcc -o $</$(EXECUTABLE) $(SOURCE_FILES_PATH) -lmingw32 -lSDL2main `pkg-config --cflags --libs sdl2`
	cp lib/SDL2.dll $< 2>/dev/null || :

.PHONY: web
web: bin/web
	@echo "Building for web:"
	emcc $(SOURCE_FILES_PATH) -o $</index.html

.PHONY: serve
serve: bin/web
	@echo "Serving web project:"
	python3 -m http.server $(SERVER_PORT) -d bin/web

.PHONY: test-all
test-all: $(TEST_TARGETS)

.PHONY: test
test-%: bin/test
	@echo "Running $* test:"
	@gcc -o $</$@ test/$*.c && $</$@ || true
	@echo ""

bin/%:
	mkdir -p $@
