
CXX := clang++
SRC := hyprgrab.cpp
TARGET := hyprgrab

INSTALL_DIR ?= /usr/bin
INSTALL_CMD := install -m 0755

# Per-target flags
CXXSTD := -std=c++20

# Release (full optimizations)
CXXFLAGS_RELEASE := $(CXXSTD) -O3 -march=native -Wall -Wextra -Wpedantic

# Debug (debug symbols + AddressSanitizer)
CXXFLAGS_DEBUG := $(CXXSTD) -O0 -g -fno-optimize-sibling-calls -Wall -Wextra -Wpedantic -fsanitize=address
LDFLAGS_DEBUG := -fsanitize=address

.PHONY: all release debug clean run help

# Default builds the optimized release
all: release

release: CXXFLAGS:=$(CXXFLAGS_RELEASE)
release: $(TARGET)

debug: CXXFLAGS:=$(CXXFLAGS_DEBUG)
debug: LDFLAGS:=$(LDFLAGS_DEBUG)
debug: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

install: release
	@echo "Installing $(TARGET) to $(INSTALL_DIR)"
	$(INSTALL_CMD) $(TARGET) $(INSTALL_DIR)/$(TARGET)

uninstall:
	@echo "Removing $(INSTALL_DIR)/$(TARGET)"
	-rm -f $(INSTALL_DIR)/$(TARGET)

clean:
	rm -f $(TARGET) *.o

# Run the debug build with ASAN runtime options (only meaningful after `make debug`)
run: debug
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./$(TARGET)

help:
	@echo "Makefile targets:"
	@echo "  make         Build release (full optimizations)"
	@echo "  make debug   Build debug ( -g + AddressSanitizer )"
	@echo "  make run     Build debug and run with ASAN runtime options"
	@echo "  make clean   Remove binary"
	@echo "  make install Install built binary to /usr/bin (or set INSTALL_DIR=/your/path)"
	@echo "  make uninstall Remove installed binary from INSTALL_DIR"

