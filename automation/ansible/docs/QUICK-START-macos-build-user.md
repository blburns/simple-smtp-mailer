# Quick Start: macOS Build User Setup

## Quick Reference Steps

### 1. Enable SSH on macOS
```bash
# System Preferences > Sharing > Remote Login (enable)
# Or via command line:
sudo systemsetup -setremotelogin on
```

### 2. Create Build User
```bash
# Create user
sudo dscl . -create /Users/build
sudo dscl . -create /Users/build UserShell /bin/zsh
sudo dscl . -create /Users/build RealName "Build Service Account"
sudo dscl . -create /Users/build UniqueID 501  # Check existing users first!
sudo dscl . -create /Users/build PrimaryGroupID 20
sudo dscl . -create /Users/build NFSHomeDirectory /Users/build

# Create home directory
sudo createhomedir -c -u build

# Set password (optional if using SSH keys only)
sudo passwd build
```

### 3. Configure Sudo Access
```bash
sudo visudo
# Add this line:
build ALL=(ALL) NOPASSWD: ALL
```

### 4. Add to Developer Group
```bash
sudo dscl . -append /Groups/_developer GroupMembership build
```

### 5. Set Up SSH Key Authentication
```bash
# On your local machine (where Ansible runs)
ssh-keygen -t ed25519 -C "ansible-build-macos" -f ~/.ssh/id_ed25519_build_macos

# Copy key to build user
ssh-copy-id -i ~/.ssh/id_ed25519_build_macos.pub build@YOUR_MAC_IP
```

### 6. Install Build Tools as Build User
```bash
# SSH into build user
ssh build@YOUR_MAC_IP

# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Add to PATH
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zshrc  # Apple Silicon
echo 'eval "$(/usr/local/bin/brew shellenv)"' >> ~/.zshrc     # Intel
source ~/.zshrc

# Install dependencies
brew install cmake openssl@3 pkg-config git

# Install Xcode Command Line Tools
xcode-select --install
```

### 7. Create Build Directory
```bash
sudo mkdir -p /opt/simple-smtp-mailer
sudo chown build:staff /opt/simple-smtp-mailer
sudo chmod 755 /opt/simple-smtp-mailer
```

### 8. Update Inventory File
Edit `automation/ansible/inventory-vmware.ini`:
- Set `ansible_host` to your Mac's IP address
- Update `ansible_ssh_private_key_file` path if different

### 9. Test Connection
```bash
# Test SSH
ssh -i ~/.ssh/id_ed25519_build_macos build@YOUR_MAC_IP

# Test Ansible
ansible BUILD_MACOS -i automation/ansible/inventory-vmware.ini -m ping
```

## Find Your Mac's IP Address

```bash
# Method 1: System Preferences
# System Preferences > Sharing > Remote Login (shows IP)

# Method 2: Command line
ipconfig getifaddr en0  # Wi-Fi
ipconfig getifaddr en1  # Ethernet

# Method 3: All interfaces
ifconfig | grep "inet " | grep -v 127.0.0.1
```

## Verify Setup

```bash
# SSH into build user and verify tools
ssh build@YOUR_MAC_IP

# Check tools
cmake --version
g++ --version
clang++ --version
git --version
brew --version
```

## Troubleshooting

**SSH connection fails:**
- Check Remote Login is enabled
- Verify firewall settings
- Check SSH key permissions: `chmod 600 ~/.ssh/id_ed25519_build_macos`

**Sudo access denied:**
- Verify visudo configuration
- Check user is in admin group: `dscl . -read /Groups/admin GroupMembership`

**Build tools not found:**
- Verify Homebrew is in PATH
- Check Xcode Command Line Tools: `xcode-select -p`

