#!/bin/sh

# Check the shell
SHELL_NAME=$(basename "$SHELL")

# Determine the shell rc file
case "$SHELL_NAME" in
    zsh)
        RC_FILE="$HOME/.zshrc"
        ;;
    bash)
        RC_FILE="$HOME/.bashrc"
        ;;
    ksh)
        RC_FILE="$HOME/.kshrc"
        ;;
    *)
        echo "Unsupported shell: $SHELL_NAME"
        exit 1
        ;;
esac

# Create build directory and build the project
cmake -S . -B build/
cmake --build build/

# Add alias to the appropriate shell rc file
echo "alias lmus=$(pwd)/build/Litemus" >> "$RC_FILE"

echo -e "\n[SUCCESS] Alias added to $RC_FILE. Please restart your terminal or source the rc file to apply the changes."
