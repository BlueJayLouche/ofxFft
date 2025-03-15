#!/bin/bash
# Build FFTW for Linux

# Detect architecture
ARCH=$(uname -m)
case $ARCH in
    x86_64)
        CFLAGS="-O3 -march=x86-64 -mtune=generic"
        ;;
    i?86)
        CFLAGS="-O3 -march=i686 -mtune=generic"
        ;;
    aarch64)
        CFLAGS="-O3 -march=armv8-a"
        ;;
    *)
        CFLAGS="-O3"
        ;;
esac

cd libs/fftw/src

# Clean previous builds
make clean || true

# Configure and build
./configure --disable-shared --enable-static --enable-float --enable-threads \
  CFLAGS="$CFLAGS"
make -j$(nproc)

# Copy libraries to the proper location
mkdir -p ../lib/linux/$ARCH/
cp .libs/libfftw3f.a ../lib/linux/$ARCH/
cp .libs/libfftw3f_threads.a ../lib/linux/$ARCH/

echo "FFTW built successfully for Linux ($ARCH)"