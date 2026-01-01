#!/bin/bash
# Installation script for simple-smtp-mailer
# This script installs the application from a DMG package

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PROJECT_NAME="simple-smtp-mailer"
INSTALL_PREFIX="/usr/local"
BIN_DIR="$INSTALL_PREFIX/bin"
LIB_DIR="$INSTALL_PREFIX/lib"
ETC_DIR="/etc/$PROJECT_NAME"
SHARE_DIR="$INSTALL_PREFIX/share/$PROJECT_NAME"

# Get the directory where this script is located (DMG mount point)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DMG_ROOT="$(dirname "$SCRIPT_DIR")"

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_root() {
    if [[ $EUID -ne 0 ]]; then
        print_error "This script must be run as root (use sudo)"
        echo ""
        echo "Usage: sudo ./install.sh"
        exit 1
    fi
}

find_application() {
    # Look for the application binary in common DMG locations
    local search_paths=(
        "$DMG_ROOT"
        "$SCRIPT_DIR"
        "$(dirname "$DMG_ROOT")"
    )
    
    for path in "${search_paths[@]}"; do
        if [[ -f "$path/$PROJECT_NAME" ]]; then
            echo "$path/$PROJECT_NAME"
            return 0
        fi
        if [[ -f "$path/bin/$PROJECT_NAME" ]]; then
            echo "$path/bin/$PROJECT_NAME"
            return 0
        fi
        if [[ -d "$path" ]]; then
            local found=$(find "$path" -name "$PROJECT_NAME" -type f -perm +111 2>/dev/null | head -1)
            if [[ -n "$found" ]]; then
                echo "$found"
                return 0
            fi
        fi
    done
    
    return 1
}

install_application() {
    print_info "Installing $PROJECT_NAME..."
    
    # Find the application binary
    local app_binary=$(find_application)
    if [[ -z "$app_binary" ]]; then
        print_error "Could not find $PROJECT_NAME binary in DMG"
        print_info "Please ensure the DMG is properly mounted and contains the application"
        exit 1
    fi
    
    print_info "Found application: $app_binary"
    
    # Create directories
    print_info "Creating installation directories..."
    mkdir -p "$BIN_DIR"
    mkdir -p "$LIB_DIR"
    mkdir -p "$ETC_DIR"
    mkdir -p "$SHARE_DIR"/{docs,oauth2-helper}
    
    # Install binary
    print_info "Installing binary to $BIN_DIR..."
    cp "$app_binary" "$BIN_DIR/$PROJECT_NAME"
    chmod 755 "$BIN_DIR/$PROJECT_NAME"
    print_success "Binary installed"
    
    # Install library if it exists
    local lib_file=$(find "$(dirname "$app_binary")" -name "lib${PROJECT_NAME}*.dylib" -o -name "lib${PROJECT_NAME}*.a" 2>/dev/null | head -1)
    if [[ -n "$lib_file" ]]; then
        print_info "Installing library to $LIB_DIR..."
        cp "$lib_file" "$LIB_DIR/"
        print_success "Library installed"
    fi
    
    # Install configuration files if they exist
    local config_dir=$(find "$(dirname "$app_binary")" -type d -name "config" -o -type d -name "etc" 2>/dev/null | head -1)
    if [[ -n "$config_dir" ]] && [[ -d "$config_dir" ]]; then
        print_info "Installing configuration files to $ETC_DIR..."
        cp -r "$config_dir"/* "$ETC_DIR/" 2>/dev/null || true
        print_success "Configuration files installed"
    fi
    
    # Install documentation if it exists
    local docs_dir=$(find "$(dirname "$app_binary")" -type d -name "docs" 2>/dev/null | head -1)
    if [[ -n "$docs_dir" ]] && [[ -d "$docs_dir" ]]; then
        print_info "Installing documentation to $SHARE_DIR/docs..."
        cp -r "$docs_dir"/* "$SHARE_DIR/docs/" 2>/dev/null || true
        print_success "Documentation installed"
    fi
    
    # Install OAuth2 helper tools if they exist
    local tools_dir=$(find "$(dirname "$app_binary")" -type d -name "oauth2-helper" 2>/dev/null | head -1)
    if [[ -n "$tools_dir" ]] && [[ -d "$tools_dir" ]]; then
        print_info "Installing OAuth2 helper tools to $SHARE_DIR/oauth2-helper..."
        cp -r "$tools_dir"/* "$SHARE_DIR/oauth2-helper/" 2>/dev/null || true
        print_success "OAuth2 helper tools installed"
    fi
    
    # Install launchd service file if it exists
    local plist_file=$(find "$(dirname "$app_binary")" -name "com.${PROJECT_NAME}.${PROJECT_NAME}.plist" 2>/dev/null | head -1)
    if [[ -n "$plist_file" ]] && [[ -f "$plist_file" ]]; then
        print_info "Installing launchd service file..."
        cp "$plist_file" "/Library/LaunchDaemons/"
        chmod 644 "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
        print_success "Service file installed"
        print_info "To start the service, run: sudo launchctl load /Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
    fi
    
    print_success "Installation completed successfully!"
    echo ""
    print_info "Installation summary:"
    echo "  Binary: $BIN_DIR/$PROJECT_NAME"
    echo "  Configuration: $ETC_DIR"
    echo "  Documentation: $SHARE_DIR/docs"
    echo "  Tools: $SHARE_DIR/oauth2-helper"
    echo ""
    print_info "To verify installation, run: $PROJECT_NAME --version"
}

main() {
    echo ""
    echo "=========================================="
    echo "  $PROJECT_NAME Installation Script"
    echo "=========================================="
    echo ""
    
    check_root
    install_application
}

main "$@"

