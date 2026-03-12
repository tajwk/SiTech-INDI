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

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build..."
cmake .. -DCMAKE_BUILD_TYPE=Release

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
