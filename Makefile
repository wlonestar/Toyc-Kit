SHELL = bash

CLANG = /usr/bin/clang-16
CLANG++ = /usr/bin/clang++-16

BUILD_DIR = build
LIB_DIR = lib
LIB_NAME = libtoyc

.DEFAULT_GOAL := build

TYPE ?= Debug

config:
	@cmake -G Ninja -B $(BUILD_DIR) \
		-DCMAKE_C_COMPILER=$(CLANG) \
    -DCMAKE_CXX_COMPILER=$(CLANG++) \
		-DCMAKE_BUILD_TYPE=$(TYPE)

build: config
	@ninja -C $(BUILD_DIR)

# compile toyc runtime (shared object)
runtime:
	@$(CLANG++) -fPIC -shared -lm -o $(LIB_DIR)/$(LIB_NAME).so $(LIB_DIR)/$(LIB_NAME).cpp
	@sudo cp $(LIB_DIR)/$(LIB_NAME).so /usr/local/lib
	@sudo ldconfig

.PHONY: clean

clean:
	@rm -rf $(BUILD_DIR)
