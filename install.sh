#!/bin/bash

# gh-repo-create Installation Script
# Installs gh-repo-create to /usr/local/bin

set -e

PREFIX="${PREFIX:-/usr/local}"
BIN_DIR="${PREFIX}/bin"
MAN_DIR="${PREFIX}/share/man/man1}"
DOC_DIR="${PREFIX}/share/doc/gh-repo-create"

echo "gh-repo-create Installer"
echo "========================"
echo ""

# Check if running as root or have sudo access
if [ "$EUID" -ne 0 ] && [ ! -w "$BIN_DIR" ]; then
    echo "Note: Installation requires root privileges."
    echo "Run with sudo or set PREFIX to a directory you can write to."
    echo ""
    echo "Example: sudo PREFIX=\$HOME/.local ./install.sh"
    exit 1
fi

# Build the project if not already built
if [ ! -f "build/gh-repo" ]; then
    echo "Building gh-repo-create..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
fi

echo "Installing gh-repo-create to $BIN_DIR..."

# Install binary
mkdir -p "$BIN_DIR"
cp build/gh-repo "$BIN_DIR/gh-repo"
chmod 755 "$BIN_DIR/gh-repo"

# Install man page
if [ -f "docs/gh-repo.1" ]; then
    echo "Installing man page..."
    mkdir -p "$MAN_DIR"
    cp docs/gh-repo.1 "$MAN_DIR/"
    chmod 644 "$MAN_DIR/gh-repo.1"
fi

# Install documentation
echo "Installing documentation..."
mkdir -p "$DOC_DIR"
cp -r docs/* "$DOC_DIR/" 2>/dev/null || true
cp README.md "$DOC_DIR/" 2>/dev/null || true

echo ""
echo "Installation complete!"
echo ""
echo "Binary: $BIN_DIR/gh-repo"
echo "Config: ~/.gh-repo-create.json"
echo "History: ~/.gh-repo-create-history"
echo ""

# Offer to create config file
if [ ! -f "$HOME/.gh-repo-create.json" ]; then
    echo "Would you like to create a config file? (y/n)"
    read -r response
    if [ "$response" = "y" ]; then
        echo '{"token": ""}' > "$HOME/.gh-repo-create.json"
        echo "Created ~/.gh-repo-create.json - edit it and add your token"
    fi
fi

echo ""
echo "Run 'gh-repo --help' to get started!"
