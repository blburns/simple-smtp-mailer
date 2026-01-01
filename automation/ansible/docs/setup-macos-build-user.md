# Setting Up macOS Build User for Ansible

This guide explains how to create a `build` service account on macOS that can be used by Ansible for remote builds.

## Overview

The `build` user will:
- Have SSH access for Ansible
- Be able to compile C++ code
- Have access to necessary build tools (Xcode, Homebrew, etc.)
- Run with appropriate permissions

## Prerequisites

- macOS with Xcode Command Line Tools installed
- Administrator access to create users
- SSH server enabled (System Preferences > Sharing > Remote Login)

## Step-by-Step Setup

### Step 1: Enable SSH on macOS

1. Open **System Preferences** (or **System Settings** on macOS Ventura+)
2. Go to **Sharing**
3. Enable **Remote Login**
4. Optionally, restrict access to specific users or allow all users
5. Note your Mac's IP address (shown in the Remote Login settings)

### Step 2: Create the Build User

```bash
# Create the build user (run as administrator)
sudo dscl . -create /Users/build
sudo dscl . -create /Users/build UserShell /bin/zsh
sudo dscl . -create /Users/build RealName "Build Service Account"
sudo dscl . -create /Users/build UniqueID 501  # Use a unique ID, check existing users first
sudo dscl . -create /Users/build PrimaryGroupID 20  # staff group
sudo dscl . -create /Users/build NFSHomeDirectory /Users/build

# Create home directory
sudo createhomedir -c -u build

# Set a password (you'll be prompted)
sudo passwd build
```

**Note:** Choose a strong password or use SSH keys only (see Step 4).

### Step 3: Configure User Permissions

```bash
# Add build user to developer group (for Xcode access)
sudo dscl . -append /Groups/_developer GroupMembership build

# Allow build user to run sudo for package installation (optional but recommended)
# Edit sudoers file
sudo visudo

# Add this line (allows build user to run commands without password):
build ALL=(ALL) NOPASSWD: ALL

# Or more restrictively, only allow specific commands:
build ALL=(ALL) NOPASSWD: /usr/bin/make, /usr/local/bin/brew, /usr/bin/git
```

### Step 4: Set Up SSH Key Authentication (Recommended)

```bash
# On your local machine (where Ansible runs), generate SSH key if needed
ssh-keygen -t ed25519 -C "ansible-build-macos" -f ~/.ssh/id_ed25519_build_macos

# Copy public key to build user on macOS
ssh-copy-id -i ~/.ssh/id_ed25519_build_macos.pub build@YOUR_MAC_IP

# Or manually:
cat ~/.ssh/id_ed25519_build_macos.pub | ssh build@YOUR_MAC_IP "mkdir -p ~/.ssh && chmod 700 ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys"
```

### Step 5: Install Build Dependencies as Build User

```bash
# SSH into the build user
ssh build@YOUR_MAC_IP

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Add Homebrew to PATH
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zshrc
echo 'eval "$(/usr/local/bin/brew shellenv)"' >> ~/.zshrc  # For Intel Macs
source ~/.zshrc

# Install build dependencies
brew install cmake openssl@3 pkg-config git

# Install Xcode Command Line Tools (if not already installed)
xcode-select --install
```

### Step 6: Configure Build Directory Permissions

```bash
# Create build directory with appropriate permissions
sudo mkdir -p /opt/simple-smtp-mailer
sudo chown build:staff /opt/simple-smtp-mailer
sudo chmod 755 /opt/simple-smtp-mailer
```

### Step 7: Test SSH Connection

```bash
# From your local machine, test SSH connection
ssh -i ~/.ssh/id_ed25519_build_macos build@YOUR_MAC_IP

# Test Ansible connectivity
ansible BUILD_MACOS -i automation/ansible/inventory-vmware.ini -m ping
```

### Step 8: Verify Build Capabilities

```bash
# SSH into build user and test compilation
ssh build@YOUR_MAC_IP

# Test CMake
cmake --version

# Test compiler
g++ --version
clang++ --version

# Test git
git --version
```

## Security Considerations

1. **SSH Key Only**: Consider disabling password authentication and using SSH keys only
2. **Firewall**: Configure macOS firewall to only allow SSH from trusted IPs
3. **Sudo Access**: Limit sudo access to only necessary commands
4. **User Isolation**: The build user should only have access to build directories
5. **Regular Updates**: Keep macOS and build tools updated

## Troubleshooting

### SSH Connection Issues

```bash
# Check SSH service status
sudo systemsetup -getremotelogin

# Enable SSH if disabled
sudo systemsetup -setremotelogin on

# Check SSH configuration
sudo cat /etc/ssh/sshd_config
```

### Permission Issues

```bash
# Check user permissions
id build
groups build

# Verify home directory ownership
ls -la /Users/build
```

### Build Tool Issues

```bash
# Verify Xcode Command Line Tools
xcode-select -p

# Reinstall if needed
sudo xcode-select --reset
xcode-select --install

# Verify Homebrew
brew doctor
```

## Next Steps

After completing these steps:

1. Update `automation/ansible/inventory-vmware.ini` with your Mac's IP address
2. Test Ansible connectivity: `ansible BUILD_MACOS -i automation/ansible/inventory-vmware.ini -m ping`
3. Run a test build: `./automation/ansible/scripts/remote-build.sh -l BUILD_MACOS`

## Additional Resources

- [macOS User Management](https://support.apple.com/guide/terminal/create-and-manage-user-accounts-apdd1003-0101)
- [SSH Configuration on macOS](https://support.apple.com/guide/remote-desktop/about-remote-login-apdbfd3b5b1)
- [Homebrew Installation](https://brew.sh)

