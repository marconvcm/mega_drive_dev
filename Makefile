# Mega Drive Development Makefile
# Based on SGDK (Sega Genesis Development Kit)

# Variables
DOCKER_IMAGE = registry.gitlab.com/doragasu/docker-sgdk:v2.00
PROJECT_NAME = game
ROM_NAME = $(PROJECT_NAME).md
BUILD_DIR = out
SGDK_LOCAL_DIR = .sgdk

# Source files
SOURCES = main.c src/boot/rom_head.c
ASM_SOURCES = src/boot/sega.s

.PHONY: all build clean sdk download-headers help run

# Default target
all: build

# Build the ROM using Docker
build:
	@echo "Building Mega Drive ROM using Docker..."
	@mkdir -p $(BUILD_DIR)
	docker run --rm -v $(PWD):/m68k -t $(DOCKER_IMAGE)
	@if [ -f "$(BUILD_DIR)/rom.bin" ]; then \
		mv $(BUILD_DIR)/rom.bin $(ROM_NAME); \
		echo "Build complete! ROM created as $(ROM_NAME)"; \
	else \
		echo "Build failed - rom.bin not found in $(BUILD_DIR)"; \
		exit 1; \
	fi

# Download SGDK headers for IntelliSense support
sdk: download-headers

download-headers:
	@echo "Downloading SGDK headers for IntelliSense..."
	@mkdir -p "$(SGDK_LOCAL_DIR)/inc"
	@if command -v curl > /dev/null 2>&1; then \
		curl -L https://github.com/Stephane-D/SGDK/archive/refs/heads/master.zip -o sgdk.zip; \
	elif command -v wget > /dev/null 2>&1; then \
		wget https://github.com/Stephane-D/SGDK/archive/refs/heads/master.zip -O sgdk.zip; \
	else \
		echo "Error: Neither curl nor wget found. Please install one of them."; \
		exit 1; \
	fi
	@unzip -j sgdk.zip "SGDK-master/inc/*" -d "$(SGDK_LOCAL_DIR)/inc" > /dev/null 2>&1 || true
	@rm -f sgdk.zip
	@echo "SGDK headers downloaded to $(SGDK_LOCAL_DIR)/inc"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -f *.bin *.elf *.out *.o *.s *.md
	@find . -name "*.o" -delete 2>/dev/null || true
	@find . -name "*.s" -delete 2>/dev/null || true
	@echo "Clean complete."

# Clean everything including downloaded headers
clean-all: clean
	@echo "Removing SGDK headers..."
	@rm -rf $(SGDK_LOCAL_DIR)
	@echo "Full clean complete."

# Pull the latest Docker image
update-docker:
	@echo "Pulling latest Docker image..."
	docker pull $(DOCKER_IMAGE)
	@echo "Docker image updated."

# Run the ROM in an emulator (requires emulator to be installed)
run: build
	@echo "Looking for available emulators..."
	@if command -v gens > /dev/null 2>&1; then \
		echo "Running with Gens..."; \
		gens $(ROM_NAME); \
	elif command -v blastem > /dev/null 2>&1; then \
		echo "Running with BlastEm..."; \
		blastem $(ROM_NAME); \
	elif command -v mednafen > /dev/null 2>&1; then \
		echo "Running with Mednafen..."; \
		mednafen $(ROM_NAME); \
	elif [ -d "/Applications/OpenEmu.app" ]; then \
		echo "Running with OpenEmu..."; \
		open -a OpenEmu $(ROM_NAME); \
	else \
		echo "No compatible emulator found."; \
		echo "Please install one of: gens, blastem, mednafen, or OpenEmu (macOS)"; \
		echo "ROM is ready at: $(ROM_NAME)"; \
	fi

# Show file information
info:
	@echo "Project: $(PROJECT_NAME)"
	@echo "ROM file: $(ROM_NAME)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Docker image: $(DOCKER_IMAGE)"
	@echo ""
	@echo "Source files:"
	@for file in $(SOURCES) $(ASM_SOURCES); do \
		if [ -f "$$file" ]; then \
			echo "  ✓ $$file"; \
		else \
			echo "  ✗ $$file (missing)"; \
		fi; \
	done
	@echo ""
	@if [ -f "$(ROM_NAME)" ]; then \
		echo "ROM status: ✓ Built ($(ROM_NAME))"; \
		ls -lh $(ROM_NAME); \
	else \
		echo "ROM status: ✗ Not built"; \
	fi

# Help target
help:
	@echo "Mega Drive Development Makefile"
	@echo "Based on SGDK (Sega Genesis Development Kit)"
	@echo ""
	@echo "Targets:"
	@echo "  build         - Build the ROM using Docker"
	@echo "  sdk           - Download SGDK headers for IntelliSense"
	@echo "  clean         - Clean build artifacts"
	@echo "  clean-all     - Clean everything including headers"
	@echo "  update-docker - Pull latest Docker image"
	@echo "  run           - Build and run ROM in emulator"
	@echo "  info          - Show project information"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make sdk      (download headers for IDE support)"
	@echo "  2. make build    (build the ROM)"
	@echo "  3. make run      (run in emulator)"
