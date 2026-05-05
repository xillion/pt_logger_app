#!/bin/bash
set -e

# Path to the generated binary
BIN_FILE="./build/zephyr/zephyr.bin"

if [ ! -f "$BIN_FILE" ]; then
    echo "Error: $BIN_FILE not found. Please build the project first."
    exit 1
fi

echo "Flashing $BIN_FILE via DFU..."
STM32_Programmer_CLI -c port=usb1 -w "$BIN_FILE" 0x08000000 -v -s
echo "Flashing complete."
