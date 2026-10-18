#!/bin/bash

# CANgaroo Dependency Installer for Linux
# Supports Ubuntu/Debian, Fedora, and Arch Linux

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
echo "\____//_/  |_/_/ |_/ \__, / \__,_/_/    \____/ \____/  "
echo "                    /____/                              "
echo -e "${NC}"
echo -e "${BLUE}Open-source CAN bus analyzer setup tool${NC}"
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
            # Package names differ across releases: Debian 13+/recent Ubuntu use
            # qt6-charts-dev / qt6-serialport-dev; older releases use libqt6*6-dev.
            apt_pick() {
                for p in "$@"; do
                    if apt-cache show "$p" >/dev/null 2>&1; then echo "$p"; return; fi
                done
                echo "$1"
            }
            CHARTS_PKG=$(apt_pick qt6-charts-dev libqt6charts6-dev)
            SERIAL_PKG=$(apt_pick qt6-serialport-dev libqt6serialport6-dev)
            sudo apt install -y \
                build-essential \
                qt6-base-dev \
                "$CHARTS_PKG" \
                "$SERIAL_PKG" \
                libnl-3-dev \
                libnl-route-3-dev \
                libgl1-mesa-dev \
                pkg-config \
                git \
                python3-dev \
                pybind11-dev \
                python3-pybind11
            ;;
        fedora)
            echo "Installing dependencies for Fedora (using dnf)..."
            sudo dnf install -y \
                gcc-c++ \
                make \
                qt6-qtbase-devel \
                qt6-qtcharts-devel \
                qt6-qtserialport-devel \
                libnl3-devel \
                mesa-libGL-devel \
                pkgconf-pkg-config \
                git \
                python3-devel \
                pybind11-devel
            ;;
        arch)
            echo "Installing dependencies for Arch Linux (using pacman)..."
            sudo pacman -S --needed --noconfirm \
                base-devel \
                qt6-base \
                qt6-charts \
                qt6-serialport \
                libnl \
                mesa \
                pkgconf \
                git \
                python \
                pybind11
            ;;
        *)
            echo -e "${RED}Error: Distribution $OS is not explicitly supported by this script.${NC}"
            echo "Please install Qt6 (base, charts, serialport) and libnl3 manually."
            exit 1
            ;;
    esac
}

build_cangaroo() {
    echo -e "${BLUE}Building CANgaroo...${NC}"
    cd src
    # Try to find qmake6, fallback to qmake if not found or if it's already qt6
    QMAKE_CMD=$(command -v qmake6 || command -v qmake)
    
    if [ -z "$QMAKE_CMD" ]; then
        echo -e "${RED}Error: qmake not found. Please ensure Qt6 is installed correctly.${NC}"
        exit 1
    fi

    # Set PKG_CONFIG_PATH for ubuntu/debian if needed
    if [[ "$OS" == "ubuntu" || "$OS" == "debian" || "$OS" == "linuxmint" ]]; then
        export PKG_CONFIG_PATH=/usr/lib/$(uname -m)-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
    fi

    $QMAKE_CMD
    make -j$(nproc)
    cd ..
}

install_to_bin() {
    echo -e "${BLUE}Installing CANgaroo to /usr/local/bin...${NC}"
    if [ -f "bin/cangaroo" ]; then
        sudo cp bin/cangaroo /usr/local/bin/
        echo -e "${GREEN}Cangaroo installed to /usr/local/bin/cangaroo${NC}"
    else
        echo -e "${RED}Error: Binary not found at bin/cangaroo. Did the build fail?${NC}"
        exit 1
    fi
}

echo "Select an option:"
echo "1) Install only dependencies"
echo "2) Install dependencies and build Cangaroo"
echo "3) Install dependencies, build Cangaroo, and install to /usr/local/bin"
echo "4) Build Cangaroo only"
read -p "Enter choice [1-4]: " choice

case $choice in
    1)
        install_deps
        ;;
    2)
        install_deps
        build_cangaroo
        ;;
    3)
        install_deps
        build_cangaroo
        install_to_bin
        ;;
    4)
        build_cangaroo
        ;;
    *)
        echo -e "${RED}Invalid choice. Exiting.${NC}"
        exit 1
        ;;
esac

echo "-------------------------------------------------------"
echo -e "${GREEN}Setup completed successfully!${NC}"
if [[ "$choice" -eq 2 || "$choice" -eq 4 ]]; then
    echo "You can run CANgaroo from: ./bin/cangaroo"
elif [[ "$choice" -eq 3 ]]; then
    echo "You can now run CANgaroo by simply typing 'cangaroo' in your terminal."
fi
echo "-------------------------------------------------------"
