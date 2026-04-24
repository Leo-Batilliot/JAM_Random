BUILD_DIR := build
BINARY_NAME := ./JamRandom
CMAKE := cmake
CPU := $(shell nproc 2>/dev/null || echo 4)

.PHONY: all build run clean fclean re debug

all: build

build:
	@mkdir -p $(BUILD_DIR)
	@[ -f $(BUILD_DIR)/CMakeCache.txt ] || $(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(CPU) -- --no-print-directory
	@echo "  built → $(BINARY_NAME)"

debug:
	@mkdir -p $(BUILD_DIR)
	@$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug 
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(CPU) -- --no-print-directory

run: build
	@$(BINARY_NAME)

clean:
	@$(CMAKE) --build $(BUILD_DIR) --target clean -- --no-print-directory 2>/dev/null || true

fclean:
	@rm -rf $(BUILD_DIR) JamRandom

re: fclean build
