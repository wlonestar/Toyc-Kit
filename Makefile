SHELL = bash

CLANG = /usr/bin/clang-16
CLANG++ = /usr/bin/clang++-16

BUILD_DIR = build
EXEC = toyc

.DEFAULT_GOAL := build

TYPE ?= Debug

config:
	@cmake -G Ninja -B $(BUILD_DIR) \
		-DCMAKE_C_COMPILER=$(CLANG) \
    -DCMAKE_CXX_COMPILER=$(CLANG++) \
		-DCMAKE_BUILD_TYPE=$(TYPE)

build: config
	@ninja -C $(BUILD_DIR)

run:
	@./build/bin/$(EXEC)

LIB_DIR = lib
LIB_NAME = libtoyc

# compile toyc runtime (shared object)
runtime:
	@$(CLANG++) -fPIC -shared -o $(LIB_DIR)/$(LIB_NAME).so $(LIB_DIR)/$(LIB_NAME).cpp
	@sudo cp $(LIB_DIR)/$(LIB_NAME).so /usr/local/lib
	@sudo ldconfig

# deprecated
test:
	@./build/bin/$(EXEC) example/a.toyc
	@clang -mllvm -opaque-pointers -o a.exe a.ll -ltoyc
	@./a.exe

.PHONY: clean test

clean:
	@rm -rf $(BUILD_DIR)
