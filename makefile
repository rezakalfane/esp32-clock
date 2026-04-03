PORT    = /dev/cu.usbmodem101
FQBN    = esp32:esp32:esp32c3
BAUD    = 115200
BUILD   = /tmp/clock_build

compile:
	./build.sh

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(BUILD)

monitor:
	arduino-cli monitor -p $(PORT) --config baudrate=$(BAUD)

flash: compile upload

.PHONY: compile upload monitor flash
