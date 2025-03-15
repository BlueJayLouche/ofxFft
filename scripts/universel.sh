#!/bin/bash
# Universal build script for ofxFft

# Detect platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    ./scripts/build_apple_silicon.sh
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    if grep -q "Raspberry Pi" /proc/device-tree/model 2>/dev/null; then
        # Raspberry Pi
        ./scripts/build_raspberrypi.sh
    else
        # Other Linux
        ./scripts/build_linux.sh
    fi
elif [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "cygwin"* ]]; then
    # Windows
    ./scripts/build_windows.bat
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

echo "Build completed!"