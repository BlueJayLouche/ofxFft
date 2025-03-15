#!/bin/bash
# Build FFTW for Raspberry Pi

# Detect Raspberry Pi version and set flags
if grep -q "ARMv7" /proc/cpuinfo; then
    CFLAGS="-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O3"
elif grep -q "ARMv8" /proc/cpuinfo; then
    CFLAGS="-mcpu=cortex-a53 -O3"
else
    CFLAGS="-march=armv6 -mfpu=vfp -mfloat-abi=hard -O3"
fi

cd libs/fftw/src

# Clean previous builds
make clean || true

# Configure and build
./configure --disable-shared --enable-static --enable-float --enable-threads \
  --host=arm-linux-gnueabihf CFLAGS="$CFLAGS"
make -j$(nproc)

# Copy libraries to the proper location
mkdir -p ../lib/rpi/
cp .libs/libfftw3f.a ../lib/rpi/
cp .libs/libfftw3f_threads.a ../lib/rpi/

echo "FFTW built successfully for Raspberry Pi"