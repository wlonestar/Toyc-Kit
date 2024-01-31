SHELL = bash

CLANG = /usr/bin/clang
CLANG++ = /usr/bin/clang++

BUILD_DIR = build
EXEC = toyc

.DEFAULT_GOAL := build

config:
	@cmake -G Ninja -B $(BUILD_DIR) \
		-DCMAKE_C_COMPILER=$(CLANG) \
    -DCMAKE_CXX_COMPILER=$(CLANG++)

build: config
	@ninja -C $(BUILD_DIR)

run:
	@./build/bin/$(EXEC)

.PHONY: clean

clean:
	@rm -rf $(BUILD_DIR)
