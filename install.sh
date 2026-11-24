#!/bin/bash

# SiTech INDI Driver - Automated Installation Script
# This script will install all dependencies, build, and install the driver

set -e  # Exit on any error

echo "🔭 SiTech INDI Driver Installation"
echo "=================================="
echo ""

# Check if we're on a supported system
if ! command -v apt-get &> /dev/null; then
    echo "❌ Error: This installer requires a Debian/Ubuntu-based system with apt-get"
    echo "   For other distributions, please install manually using the documentation"
    exit 1
fi

# Check for sudo access
if ! sudo -n true 2>/dev/null; then
    echo "🔑 This installer requires sudo access to install system packages"
    echo "   You may be prompted for your password"
    echo ""
fi

echo "📦 Installing system dependencies..."
sudo apt-get update -qq
# libindi-dev: headers/libraries to compile the driver
# indi-bin:    indiserver, needed to run the driver
sudo apt-get install -y libindi-dev indi-bin libnova-dev cmake build-essential pkg-config git

echo "✅ Dependencies installed successfully"
echo ""

echo "🔨 Building SiTech INDI driver..."
chmod +x build.sh
./build.sh

if [ $? -ne 0 ]; then
    echo "❌ Build failed. Please check the output above for errors."
    exit 1
fi

echo "📥 Installing driver to system..."
cd build
sudo make install

if [ $? -ne 0 ]; then
    echo "❌ Installation failed. Please check the output above for errors."
    exit 1
fi

echo ""
echo "🔧 Setting up device permissions..."
if [ -f "../99-sitech.rules" ]; then
    sudo cp ../99-sitech.rules /etc/udev/rules.d/
    sudo udevadm control --reload-rules
    echo "✅ Device permissions configured"
else
    echo "⚠️  Device permission rules not found (this is OK for TCP-only operation)"
fi

# Older versions of this installer put the driver under /usr/local. Leftover copies
# there can run in place of the new driver (/usr/local/bin is usually earlier in PATH).
if [ "${PREFIX:-/usr}" != "/usr/local" ]; then
    STALE=""
    for f in /usr/local/bin/indi_sitech_mount /usr/local/share/indi/indi_sitech.xml; do
        if [ -e "$f" ]; then
            STALE="$STALE $f"
        fi
    done
    if [ -n "$STALE" ]; then
        echo ""
        echo "⚠️  Found an older install under /usr/local. Remove it so it can't be used instead of this one:"
        echo "   sudo rm$STALE"
    fi
fi

echo ""
echo "🎉 Installation Complete!"
echo ""
echo "To start using the driver:"
echo "  1. Make sure SiTechExe is running (if using TCP)"
echo "  2. Start the driver: indiserver -v indi_sitech_mount"
echo "  3. Connect with KStars/Ekos or other INDI clients"
echo ""
echo "Ports (two separate connections):"
echo "  - INDI clients (KStars/Ekos) connect to indiserver on port 7624"
echo "  - The driver connects to SiTechExe on port 8079 (SiTechExe's \"Indi Port Number\")"
echo "  Do not enter 8079 as the INDI server port in Ekos."
echo ""
echo "For help and documentation, see:"
echo "  - README.md"
echo "  - INSTALL_GUIDE.md"
echo "  - RPI_GUIDE.md (Raspberry Pi)"
echo ""
