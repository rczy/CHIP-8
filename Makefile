EXECUTABLE= chip-8
SOURCE_FILES= main.c
SOURCE_FILES_PATH= $(addprefix src/,$(SOURCE_FILES))
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
	@echo "  test-<file>   build and run test from 'test' folder"

.PHONY: clean
clean:
	@echo "Cleaning up 'bin' directory:"
	rm -rf ./bin

.PHONY: all
all: linux

.PHONY: linux
linux: bin/linux
	@echo "Building for Linux:"
	gcc -o $</$(EXECUTABLE) $(SOURCE_FILES_PATH)

.PHONY: windows
windows: bin/windows
	@echo "Building for Windows:"
	x86_64-w64-mingw32-gcc -o $</$(EXECUTABLE) $(SOURCE_FILES_PATH)

.PHONY: web
web: bin/web
	@echo "Building for web:"
	emcc $(SOURCE_FILES_PATH) -o $</index.html

.PHONY: serve
serve: bin/web
	@echo "Serving web project:"
	python3 -m http.server $(SERVER_PORT) -d bin/web

.PHONY: test
test-%: bin/test
	gcc -o $</$@ test/$*.c && $</$@

bin/%:
	mkdir -p $@
