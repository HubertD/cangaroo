#!/bin/bash

# CANgaroo Release Setup Tool
# Use this script to install dependencies for the pre-compiled CANgaroo binary.

set -e

# Setup colors
BLUE='\033[0;34m'
CYAN='\033[0;36m'
GREEN='\033[0;32m'
RED='\033[0;31m'
ORANGE='\033[38;5;208m'
NC='\033[0m' # No Color

# Print ASCII Art
echo -e "${ORANGE}"
echo "   ______ ___    _   __                                 "
echo "  / ____//   |  / | / /____ _ ____ _ _____ ____   ____  "
echo " / /    / /| | /  |/ // __ \`// __ \`// ___// __ \ / __ \ "
echo "/ /___ / ___ |/ /|  // /_/ // /_/ // /   / /_/ // /_/ / "
echo "\____//_/  |_/_/ |_/ \__, / \__,_|_|  \___/ \___/ \___/  "
echo "                    /____/                              "
echo -e "${NC}"
echo -e "${BLUE}CANgaroo Release Setup Tool${NC}"
echo "-------------------------------------------------------"

# Detect Distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo -e "${RED}Error: Unsupported Linux distribution.${NC}"
    exit 1
fi

echo -e "Detected OS: ${GREEN}$OS${NC}"

install_deps() {
    case $OS in
        ubuntu|debian|linuxmint)
            echo "Installing dependencies for $OS (using apt)..."
            sudo apt update
            sudo apt install -y \
                qt6-base-dev \
                libqt6charts6-dev \
                libqt6serialport6-dev \
                libnl-3-dev \
                libnl-route-3-dev \
                libgl1-mesa-dev
            ;;
        fedora)
            echo "Installing dependencies for Fedora (using dnf)..."
            sudo dnf install -y \
                qt6-qtbase-devel \
                qt6-qtcharts-devel \
                qt6-qtserialport-devel \
                libnl3-devel \
                mesa-libGL-devel
            ;;
        arch)
            echo "Installing dependencies for Arch Linux (using pacman)..."
            sudo pacman -S --needed --noconfirm \
                qt6-base \
                qt6-charts \
                qt6-serialport \
                libnl \
                mesa
            ;;
        *)
            echo -e "${RED}Error: Distribution $OS is not explicitly supported by this script.${NC}"
            echo "Please install Qt6 (base, charts, serialport) and libnl3 manually."
            exit 1
            ;;
    esac
}

install_to_bin() {
    echo -e "${BLUE}Installing CANgaroo to /usr/local/bin...${NC}"
    if [ -f "cangaroo" ]; then
        sudo cp cangaroo /usr/local/bin/
        echo -e "${GREEN}Cangaroo installed to /usr/local/bin/cangaroo${NC}"
    else
        echo -e "${RED}Error: Binary 'cangaroo' not found in current directory.${NC}"
        exit 1
    fi
}

echo "Select an option:"
echo "1) Install only dependencies"
echo "2) Install dependencies and move CANgaroo to /usr/local/bin"
read -p "Enter choice [1-2]: " choice

case $choice in
    1)
        install_deps
        ;;
    2)
        install_deps
        install_to_bin
        ;;
    *)
        echo -e "${RED}Invalid choice. Exiting.${NC}"
        exit 1
        ;;
esac

echo "-------------------------------------------------------"
echo -e "${GREEN}Setup completed successfully!${NC}"
if [[ "$choice" -eq 1 ]]; then
    echo "You can run CANgaroo from the current directory: ./cangaroo"
elif [[ "$choice" -eq 2 ]]; then
    echo "You can now run CANgaroo by simply typing 'cangaroo' in your terminal."
fi
echo "-------------------------------------------------------"
