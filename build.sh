#!/bin/bash

# SiTech INDI Driver Build Script

echo "Building SiTech INDI Driver..."

# Check for required dependencies
echo "Checking dependencies..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is required but not installed."
    echo "Install with: sudo apt-get install cmake"
    exit 1
fi

# Check for INDI development files
if ! pkg-config --exists libindi; then
    echo "Error: INDI development libraries not found."
    echo "Install with: sudo apt-get install libindi-dev"
    exit 1
fi

# Check for libnova
if pkg-config --exists libnova; then
    echo "libnova found via pkg-config."
elif pkg-config --exists nova; then
    echo "libnova found via pkg-config (nova)."
elif [ -f "/usr/include/libnova/libnova.h" ]; then
    echo "libnova found via headers."
else
    echo "Error: libnova development libraries not found."
    echo "Install with: sudo apt-get install libnova-dev"
    exit 1
fi

echo "All dependencies found."

# indiserver is not needed to compile, but it is needed to run the driver
if ! command -v indiserver &> /dev/null; then
    echo "Warning: indiserver not found. The driver will build, but you need indiserver to run it."
    echo "Install with: sudo apt-get install indi-bin"
fi

# Install prefix. /usr puts the driver XML in /usr/share/indi, where KStars/Ekos
# and INDI Web Manager look for it. Override with: PREFIX=/some/path ./build.sh
# Passed explicitly so an existing build directory configured for /usr/local is updated.
INSTALL_PREFIX="${PREFIX:-/usr}"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build (install prefix: $INSTALL_PREFIX)..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"

if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed."
    exit 1
fi

# Build the driver
echo "Building driver..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Error: Build failed."
    exit 1
fi

echo "Build completed successfully!"
echo ""
echo "To install the driver, run:"
echo "    sudo make install"
echo ""
echo "To create a package, run:"
echo "    make package"
