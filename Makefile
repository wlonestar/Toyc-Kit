SHELL = bash

CLANG = /usr/bin/clang-16
CLANG++ = /usr/bin/clang++-16

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

test:
	@./build/bin/$(EXEC) example/a.toyc
	@clang -mllvm -opaque-pointers a.ll -o a.exe

.PHONY: clean test

clean:
	@rm -rf $(BUILD_DIR)
