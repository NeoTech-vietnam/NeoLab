.PHONY: all build flash monitor flash-monitor full menuconfig set-target clean host-test check-env check-port

TARGET ?= esp32s3
BUILD_DIR ?= build-$(TARGET)
# Export ESP-IDF 6.0.1 first. No machine-specific SDK installation discovery.
IDF ?= python "$(IDF_PATH)/tools/idf.py"

all: build

check-env:
	@test -n "$(IDF_PATH)" || (echo "Export the ESP-IDF 6.0.1 environment first"; exit 1)

check-port:
	@test -n "$(PORT)" || (echo "Set PORT explicitly, e.g. make PORT=/dev/ttyACM0 flash-monitor"; exit 1)

build: check-env
	$(IDF) -B $(BUILD_DIR) build

set-target: check-env
	$(IDF) -B $(BUILD_DIR) set-target $(TARGET)

menuconfig: check-env
	$(IDF) -B $(BUILD_DIR) menuconfig

flash: check-env check-port
	$(IDF) -B $(BUILD_DIR) -p $(PORT) flash

monitor: check-env check-port
	$(IDF) -B $(BUILD_DIR) -p $(PORT) monitor

flash-monitor: check-env check-port
	$(IDF) -B $(BUILD_DIR) -p $(PORT) flash monitor

full: check-env check-port
	$(IDF) -B $(BUILD_DIR) -p $(PORT) build flash monitor

clean: check-env
	$(IDF) -B $(BUILD_DIR) fullclean

host-test:
	python3 tests/run_host_tests.py
