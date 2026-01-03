# Linux Installation Guide

This guide covers installing simple-smtp-mailer on Linux systems using various methods and distributions.

## 🚀 Quick Installation

### Ubuntu/Debian (Recommended)
```bash
# Download latest .deb package
curl -L -o simple-smtp-mailer.deb "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_amd64.deb"

# Install package
sudo dpkg -i simple-smtp-mailer.deb

# Fix dependencies if needed
sudo apt install -f

# Verify installation
simple-smtp-mailer --version
```

### CentOS/RHEL/Rocky Linux
```bash
# Download latest .rpm package
curl -L -o simple-smtp-mailer.rpm "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_x86_64.rpm"

# Install package
sudo rpm -i simple-smtp-mailer.rpm

# Verify installation
simple-smtp-mailer --version
```

### Generic Linux (.tar.gz)
```bash
# Download latest .tar.gz package
curl -L -o simple-smtp-mailer.tar.gz "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_linux_x86_64.tar.gz"

# Extract package
tar -xzf simple-smtp-mailer.tar.gz

# Move to system directory
sudo mv simple-smtp-mailer /usr/local/bin/

# Verify installation
simple-smtp-mailer --version
```

## 📦 Package Installation

### Ubuntu/Debian (.deb)

#### Download and Install
```bash
# Download latest release
curl -L -o simple-smtp-mailer.deb "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_amd64.deb"

# Install package
sudo dpkg -i simple-smtp-mailer.deb

# Fix any dependency issues
sudo apt install -f
```

#### Verify Installation
```bash
# Check version
simple-smtp-mailer --version

# Check binary location
which simple-smtp-mailer

# Check package status
dpkg -l | grep simple-smtp-mailer
```

#### Update Package
```bash
# Remove old version
sudo dpkg -r simple-smtp-mailer

# Install new version
sudo dpkg -i simple-smtp-mailer.deb
sudo apt install -f
```

### CentOS/RHEL/Rocky Linux (.rpm)

#### Download and Install
```bash
# Download latest release
curl -L -o simple-smtp-mailer.rpm "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_x86_64.rpm"

# Install package
sudo rpm -i simple-smtp-mailer.rpm

# Check for conflicts
rpm -q simple-smtp-mailer
```

#### Verify Installation
```bash
# Check version
simple-smtp-mailer --version

# Check binary location
which simple-smtp-mailer

# Check package info
rpm -qi simple-smtp-mailer
```

#### Update Package
```bash
# Upgrade package
sudo rpm -U simple-smtp-mailer.rpm

# Or remove and reinstall
sudo rpm -e simple-smtp-mailer
sudo rpm -i simple-smtp-mailer.rpm
```

### Alpine Linux (.tar.gz)

#### Download and Install
```bash
# Download latest release
curl -L -o simple-smtp-mailer.tar.gz "https://github.com/blburns/simple-smtp-mailer/releases/latest/download/simple-smtp-mailer_linux_x86_64.tar.gz"

# Extract package
tar -xzf simple-smtp-mailer.tar.gz

# Move to system directory
sudo mv simple-smtp-mailer /usr/local/bin/

# Make executable
sudo chmod +x /usr/local/bin/simple-smtp-mailer
```

#### Verify Installation
```bash
# Check version
simple-smtp-mailer --version

# Check binary location
which simple-smtp-mailer
```

## 🛠️ Build from Source

### Prerequisites

#### Required Dependencies
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git

# CentOS/RHEL/Rocky Linux
sudo yum groupinstall "Development Tools"
sudo yum install cmake git
# OR for newer versions:
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# Arch Linux
sudo pacman -S base-devel cmake git

# Alpine Linux
apk add build-base cmake git
```

#### Development Libraries
```bash
# Ubuntu/Debian
sudo apt install libssl-dev libcrypto++-dev

# CentOS/RHEL/Rocky Linux
sudo yum install openssl-devel
# OR for newer versions:
sudo dnf install openssl-devel

# Fedora
sudo dnf install openssl-devel

# Arch Linux
sudo pacman -S openssl

# Alpine Linux
apk add openssl-dev
```

#### Optional Dependencies
```bash
# Ubuntu/Debian
sudo apt install pkg-config libsystemd-dev

# CentOS/RHEL/Rocky Linux
sudo yum install pkgconfig systemd-devel
# OR for newer versions:
sudo dnf install pkgconfig systemd-devel

# Fedora
sudo dnf install pkgconfig systemd-devel

# Arch Linux
sudo pacman -S pkgconf systemd

# Alpine Linux
apk add pkgconfig
```

### Build Methods

#### Method 1: Using Build Scripts (Recommended)
```bash
# Clone the repository
git clone https://github.com/blburns/simple-smtp-mailer.git
cd simple-smtp-mailer

# Make build script executable
chmod +x scripts/build-linux.sh

# Run the build script
./scripts/build-linux.sh
```

#### Build with Options
```bash
# Build with debug symbols
./scripts/build-linux.sh --debug

# Build with specific CMake options
./scripts/build-linux.sh --cmake-options "-DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON"

# Build for specific architecture
./scripts/build-linux.sh --arch x86_64
```

#### Method 2: Manual CMake Build
```bash
# Clone the repository
git clone https://github.com/blburns/simple-smtp-mailer.git
cd simple-smtp-mailer

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
make -j$(nproc)

# Install system-wide
sudo make install
```

### Build Configuration Options

#### CMake Options
```bash
# Build type
-DCMAKE_BUILD_TYPE=Release    # Release build (default)
-DCMAKE_BUILD_TYPE=Debug      # Debug build with symbols
-DCMAKE_BUILD_TYPE=RelWithDebInfo  # Release with debug info

# Architecture
-DCMAKE_SYSTEM_PROCESSOR=x86_64   # Target x86_64
-DCMAKE_SYSTEM_PROCESSOR=aarch64  # Target ARM64

# Features
-DENABLE_TESTS=ON             # Enable test suite
-DENABLE_DOCS=ON              # Build documentation
-DUSE_SYSTEM_LIBS=ON          # Use system libraries
```

#### Compiler Options
```bash
# Set compiler
export CC=gcc
export CXX=g++

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
dpkg -l | grep simple-smtp-mailer  # Ubuntu/Debian
rpm -q simple-smtp-mailer          # CentOS/RHEL
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
# Check dependencies
sudo apt install build-essential cmake  # Ubuntu/Debian
sudo yum groupinstall "Development Tools"  # CentOS/RHEL

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

### Debug Commands
```bash
# Check system information
uname -a
cat /etc/os-release

# Check installed packages
dpkg -l | grep simple-smtp-mailer  # Ubuntu/Debian
rpm -qa | grep simple-smtp-mailer  # CentOS/RHEL

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

*simple-smtp-mailer is now installed on your Linux system! Next, set up OAuth2 authentication and configure your email relay system.*
