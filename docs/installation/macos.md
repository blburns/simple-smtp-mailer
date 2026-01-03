# macOS Installation Guide

This guide covers installing simple-smtp-mailer on macOS systems using various methods and supporting both Intel and Apple Silicon Macs.

## 🚀 Quick Installation

### DMG Installer (Recommended)
```bash
# Download latest DMG package
curl -L -o simple-smtp-mailer.dmg "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_macos.dmg"

# Mount DMG
hdiutil attach simple-smtp-mailer.dmg

# Install package
sudo installer -pkg /Volumes/simple-smtp-mailer/simple-smtp-mailer.pkg -target /

# Unmount DMG
hdiutil detach /Volumes/simple-smtp-mailer

# Verify installation
simple-smtp-mailer --version
```

### Homebrew Installation
```bash
# Add custom tap (if available)
brew tap blburns/simple-smtp-mailer

# Install simple-smtp-mailer
brew install simple-smtp-mailer

# Verify installation
simple-smtp-mailer --version
```

### PKG Installer
```bash
# Download latest PKG package
curl -L -o simple-smtp-mailer.pkg "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_macos.pkg"

# Install package
sudo installer -pkg simple-smtp-mailer.pkg -target /

# Verify installation
simple-smtp-mailer --version
```

## 📦 Package Installation

### DMG Installer

#### Download and Install
```bash
# Download latest release
curl -L -o simple-smtp-mailer.dmg "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_macos.dmg"

# Mount DMG
hdiutil attach simple-smtp-mailer.dmg

# Install package
sudo installer -pkg /Volumes/simple-smtp-mailer/simple-smtp-mailer.pkg -target /

# Unmount DMG
hdiutil detach /Volumes/simple-smtp-mailer
```

#### Verify Installation
```bash
# Check version
simple-smtp-mailer --version

# Check binary location
which simple-smtp-mailer

# Check package info
pkgutil --info com.simple-smtp-mailer.simple-smtp-mailer
```

#### Update Package
```bash
# Remove old version
sudo pkgutil --forget com.simple-smtp-mailer.simple-smtp-mailer

# Install new version
sudo installer -pkg simple-smtp-mailer.pkg -target /
```

### PKG Installer

#### Download and Install
```bash
# Download latest release
curl -L -o simple-smtp-mailer.pkg "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_macos.pkg"

# Install package
sudo installer -pkg simple-smtp-mailer.pkg -target /

# Verify installation
simple-smtp-mailer --version
```

#### Verify Installation
```bash
# Check version
simple-smtp-mailer --version

# Check binary location
which simple-smtp-mailer

# Check package info
pkgutil --info com.simple-smtp-mailer.simple-smtp-mailer
```

### Homebrew Installation

#### Install via Homebrew
```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Add Homebrew to PATH
if [[ "$(uname -m)" == "arm64" ]]; then
    eval "$(/opt/homebrew/bin/brew shellenv)"
else
    eval "$(/usr/local/bin/brew shellenv)"
fi

# Install simple-smtp-mailer
brew install simple-smtp-mailer
```

#### Verify Installation
```bash
# Check version
simple-smtp-mailer --version

# Check binary location
which simple-smtp-mailer

# Check Homebrew info
brew info simple-smtp-mailer
```

#### Update via Homebrew
```bash
# Update Homebrew
brew update

# Upgrade simple-smtp-mailer
brew upgrade simple-smtp-mailer
```

## 🛠️ Build from Source

### Prerequisites

#### System Requirements
- **macOS Version**: Big Sur (11.0) or later
- **Architecture**: Intel (x86_64) or Apple Silicon (arm64)
- **RAM**: 4GB minimum, 8GB recommended
- **Disk Space**: 2GB for build, 1GB for installation

#### Xcode Command Line Tools
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Verify installation
xcode-select -p
# Should output: /Library/Developer/CommandLineTools
```

#### Homebrew Dependencies
```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Add Homebrew to PATH
if [[ "$(uname -m)" == "arm64" ]]; then
    eval "$(/opt/homebrew/bin/brew shellenv)"
else
    eval "$(/usr/local/bin/brew shellenv)"
fi

# Install required dependencies
brew install cmake openssl@3 pkg-config

# Verify installations
cmake --version
pkg-config --version
openssl version
```

### Build Methods

#### Method 1: Using Makefile (Recommended)
```bash
# Clone the repository
git clone https://github.com/blburns/simple-smtp-mailer.git
cd simple-smtp-mailer

# Install dependencies
make deps

# Build universal binary (Intel + Apple Silicon)
make build-universal

# Or build for specific architecture
make build-intel      # Intel Macs only
make build-arm64      # Apple Silicon Macs only
```

#### Method 2: Using macOS Build Script
```bash
# Clone the repository
git clone https://github.com/blburns/simple-smtp-mailer.git
cd simple-smtp-mailer

# Make script executable
chmod +x scripts/build-macos.sh

# Build with default settings (universal binary)
./scripts/build-macos.sh

# Build with specific options
./scripts/build-macos.sh -d                    # Debug build
./scripts/build-macos.sh -a intel              # Intel only
./scripts/build-macos.sh -a arm64              # Apple Silicon only
./scripts/build-macos.sh --package             # Build and create DMG package
./scripts/build-macos.sh --help                # Show all options
```

#### Method 3: Manual CMake Build
```bash
# Clone the repository
git clone https://github.com/blburns/simple-smtp-mailer.git
cd simple-smtp-mailer

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
make -j$(sysctl -n hw.ncpu)

# Install system-wide
sudo make install
```

### Build Configuration Options

#### Architecture Options
```bash
# Universal binary (Intel + Apple Silicon)
-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# Intel only
-DCMAKE_OSX_ARCHITECTURES="x86_64"

# Apple Silicon only
-DCMAKE_OSX_ARCHITECTURES="arm64"
```

#### macOS Specific Options
```bash
# Deployment target
-DCMAKE_OSX_DEPLOYMENT_TARGET="12.0"

# SDK path
-DCMAKE_OSX_SYSROOT="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"

# Build type
-DCMAKE_BUILD_TYPE=Release
```

#### Compiler Options
```bash
# Set compiler
export CC=clang
export CXX=clang++

# Set compiler flags
export CFLAGS="-O2 -march=native"
export CXXFLAGS="-O2 -march=native"

# Configure with custom flags
cmake .. -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS"
```

## 🔧 Post-Installation Setup

### Create Required Directories
```bash
# Create configuration and log directories
sudo mkdir -p /etc/simple-smtp-mailer
sudo mkdir -p /var/log/simple-smtp-mailer
sudo mkdir -p /var/spool/simple-smtp-mailer

# Set ownership
sudo chown $USER:$USER /etc/simple-smtp-mailer
sudo chown $USER:$USER /var/log/simple-smtp-mailer
sudo chown $USER:$USER /var/spool/simple-smtp-mailer

# Set permissions
sudo chmod 755 /etc/simple-smtp-mailer
sudo chmod 755 /var/log/simple-smtp-mailer
sudo chmod 755 /var/spool/simple-smtp-mailer
```

### Install OAuth2 Helper Tools
```bash
# Install Python dependencies
pip3 install requests

# Verify OAuth2 helper
python3 tools/oauth2-helper/oauth2-helper.py --list

# Set up Gmail OAuth2
python3 tools/oauth2-helper/oauth2-helper.py gmail
```

### Basic Configuration
```bash
# Create configuration file
cat > /etc/simple-smtp-mailer/config.json << 'EOF'
{
  "smtp": {
    "host": "smtp.gmail.com",
    "port": 587,
    "security": "tls",
    "auth": {
      "type": "oauth2",
      "client_id": "YOUR_CLIENT_ID",
      "client_secret": "YOUR_CLIENT_SECRET",
      "refresh_token": "YOUR_REFRESH_TOKEN",
      "user": "your-email@gmail.com"
    }
  }
}
EOF

# Set secure permissions
chmod 600 /etc/simple-smtp-mailer/config.json
```

## 🧪 Testing Installation

### Basic Tests
```bash
# Test binary
simple-smtp-mailer --version
simple-smtp-mailer --help

# Test configuration
simple-smtp-mailer test --config /etc/simple-smtp-mailer/config.json

# Test SMTP connection
simple-smtp-mailer test --config /etc/simple-smtp-mailer/config.json --smtp
```

### Email Tests
```bash
# Send test email
simple-smtp-mailer send \
  --config /etc/simple-smtp-mailer/config.json \
  --from "your-email@gmail.com" \
  --to "test@example.com" \
  --subject "Test from simple-smtp-mailer" \
  --body "Hello from simple-smtp-mailer!"

# Test with HTML
simple-smtp-mailer send \
  --config /etc/simple-smtp-mailer/config.json \
  --from "your-email@gmail.com" \
  --to "test@example.com" \
  --subject "HTML Test" \
  --body "<h1>Hello World</h1>" \
  --html
```

## 🚨 Troubleshooting

### Common Issues

#### "Command not found: simple-smtp-mailer"
```bash
# Check if binary exists
ls -la /usr/local/bin/simple-smtp-mailer

# Add to PATH
export PATH="/usr/local/bin:$PATH"

# Check package installation
pkgutil --info com.simple-smtp-mailer.simple-smtp-mailer
```

#### "Permission denied"
```bash
# Fix ownership
sudo chown $USER:$USER /etc/simple-smtp-mailer/config.json

# Fix permissions
chmod 600 /etc/simple-smtp-mailer/config.json
chmod 755 /usr/local/bin/simple-smtp-mailer
```

#### "Build failed"
```bash
# Check Xcode Command Line Tools
xcode-select --install

# Check Homebrew dependencies
brew install cmake openssl@3 pkg-config

# Check CMake version
cmake --version  # Should be 3.16+

# Clean build directory
rm -rf build
mkdir build && cd build
```

#### "OAuth2 authentication failed"
```bash
# Check Python dependencies
pip3 install requests

# Verify OAuth2 helper
python3 tools/oauth2-helper/oauth2-helper.py --list

# Regenerate tokens
python3 tools/oauth2-helper/oauth2-helper.py gmail
```

#### "Architecture mismatch"
```bash
# Check your Mac's architecture
uname -m

# Build for your specific architecture
if [[ "$(uname -m)" == "arm64" ]]; then
    ./scripts/build-macos.sh -a arm64
else
    ./scripts/build-macos.sh -a intel
fi
```

### Debug Commands
```bash
# Check system information
uname -a
sw_vers

# Check architecture
uname -m
arch

# Check installed packages
pkgutil --info com.simple-smtp-mailer.simple-smtp-mailer

# Check file locations
which simple-smtp-mailer
ls -la /usr/local/bin/simple-smtp-mailer

# Check configuration
cat /etc/simple-smtp-mailer/config.json
```

## 📚 Next Steps

### Configuration
- **[Configuration Overview](../configuration/dns-setup.md)** - Understanding configuration options
- **[SSL/TLS Setup](../configuration/ssl-setup-with-certbot.md)** - Secure email transmission
- **[DNS Configuration](../configuration/dns-setup.md)** - Email deliverability

### OAuth2 Setup
- **[OAuth2 Overview](../oauth2/README.md)** - Complete authentication guide
- **[Gmail Setup](../oauth2/gmail-oauth2-setup.md)** - Gmail OAuth2 configuration
- **[Office 365 Setup](../oauth2/office365-oauth2-setup.md)** - Office 365 OAuth2 configuration

### Architecture
- **[Architecture Overview](../configuration/relay-architecture.md)** - System design and components
- **[Relay Setup](../configuration/relay-setup-guide.md)** - Email relay configuration

## 🆘 Getting Help

### Installation Issues
- **Check prerequisites** in [Prerequisites Guide](../getting-started/prerequisites.md)
- **Verify system requirements** and dependencies
- **Review troubleshooting** sections above

### Configuration Issues
- **Use OAuth2 helpers** for authentication setup
- **Check configuration** syntax and format
- **Review logs** for error messages

### General Support
- **GitHub Issues**: Report bugs and request features
- **GitHub Discussions**: Ask questions and share solutions
- **Documentation**: Comprehensive guides and examples

---

*simple-smtp-mailer is now installed on your macOS system! Next, set up OAuth2 authentication and configure your email relay system.*
