#!/bin/bash

set -e

PROJECT_NAME="gh-repo"
BUILD_DIR="build"

detect_os() {
    case "$(uname -s)" in
        Linux*)     echo "linux";;
        Darwin*)    echo "macos";;
        CYGWIN*|MINGW*|MSYS*) echo "windows";;
        *)          echo "unknown";;
    esac
}

install_deps_linux() {
    echo "Installing dependencies for Linux..."
    if command -v apt-get &> /dev/null; then
        sudo apt-get update
        sudo apt-get install -y cmake build-essential libreadline-dev pkg-config
    elif command -v dnf &> /dev/null; then
        sudo dnf install -y cmake gcc-c++ readline-devel
    elif command -v pacman &> /dev/null; then
        sudo pacman -S --noconfirm cmake base-devel readline
    fi
}

install_deps_macos() {
    echo "Installing dependencies for macOS..."
    if command -v brew &> /dev/null; then
        if ! command -v cmake &> /dev/null; then
            brew install cmake
        fi
        if ! command -v pkg-config &> /dev/null; then
            brew install pkg-config
        fi
    else
        echo "Error: Homebrew not found. Please install from https://brew.sh"
        exit 1
    fi
}

install_deps_windows() {
    echo "Windows detected."
    if command -v choco &> /dev/null; then
        choco install cmake pkgconfigj --installargs ADD_CMAKE_TO_PATH=System
    elif command -v winget &> /dev/null; then
        winget install -e --id Kitware.CMake
        echo "Note: Install readline via vcpkg or manual build"
    else
        echo "Please install CMake manually from https://cmake.org/download/"
        exit 1
    fi
}

install_deps() {
    local os=$(detect_os)
    case $os in
        linux)  install_deps_linux;;
        macos)  install_deps_macos;;
        windows) install_deps_windows;;
        *)      echo "Unsupported OS"; exit 1;;
    esac
}

configure() {
    local os=$(detect_os)
    echo "Configuring for $os..."
    
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    if [ "$os" = "windows" ]; then
        cmake -G "NMake Makefiles" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
    else
        cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
    fi
}

build() {
    cd "$BUILD_DIR"
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
}

install() {
    cd "$BUILD_DIR"
    make install
}

clean() {
    rm -rf "$BUILD_DIR"
    echo "Cleaned build directory."
}

show_help() {
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  install-deps  Install build dependencies"
    echo "  configure     Configure the build (implies install-deps)"
    echo "  build         Build the project"
    echo "  all           Install deps, configure, and build (default)"
    echo "  install       Install the binary"
    echo "  clean         Remove build directory"
    echo "  help          Show this help"
    echo ""
    echo "Examples:"
    echo "  $0              # Install deps, configure, and build"
    echo "  $0 install-deps # Install dependencies only"
    echo "  $0 build        # Build only"
}

case "${1:-all}" in
    install-deps)
        install_deps
        ;;
    configure)
        install_deps
        configure
        ;;
    build)
        build
        ;;
    all)
        install_deps
        configure
        build
        ;;
    install)
        install
        ;;
    clean)
        clean
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo "Unknown command: $1"
        show_help
        exit 1
        ;;
esac

echo "Done!"
