# Centralized Release Workflow

This document explains how to use the centralized release workflow for simple-smtp-mailer, which allows you to collect packages from multiple build systems and release them all at once.

## Overview

The centralized release workflow consists of three main components:

1. **Remote Build** (`automation/ansible/scripts/remote-build.sh`) - Build packages on remote VMs
2. **Package Collection** (`automation/ansible/scripts/collect-packages.sh`) - Collect packages from remote VMs
3. **Centralized Release** (`scripts/centralized-release.sh`) - Create GitHub release from centralized packages

## Workflow Steps

### Step 1: Build Packages on Remote VMs

Use Ansible to build packages on remote VMs:

```bash
# Build packages on all remote VMs
cd automation/ansible
./scripts/remote-build.sh --packages

# Or build on specific VMs
ansible-playbook -i inventory-vmware.ini playbook-build.yml \
  --limit BUILD_MACOS --extra-vars "create_packages=true"
```

This will:
- Build the project on remote VMs
- Create packages (DEB, RPM, DMG, PKG, source)
- Run `organize-packages.sh` to move packages to `dist/centralized/v{VERSION}/` on each VM

### Step 2: Collect Packages from Remote VMs

Collect packages from all remote VMs to your local system:

```bash
# Collect packages from all VMs
cd automation/ansible
./scripts/collect-packages.sh
```

This will:
- Fetch packages from `dist/centralized/v{VERSION}/` on each VM (preferred)
- Fallback to `dist/` and `build/` directories if centralized doesn't exist
- Organize packages locally into `dist/centralized/v{VERSION}/`
- Remove hostname prefixes from package names
- Display a summary of collected packages

### Step 3: Create GitHub Release

Create or update the GitHub release:

```bash
# Create/update GitHub release
cd ../..
./scripts/centralized-release.sh --release
```

This will:
- Find packages in `dist/centralized/v{VERSION}/`
- Create comprehensive release notes
- Create or update GitHub release
- Upload all packages to the release
- Update Git tag to current HEAD if needed

## Directory Structure

```
simple-smtp-mailer/
├── dist/                        # Standard distribution directory
│   ├── linux/                   # Linux packages (temporary staging)
│   │   ├── deb/                 # DEB packages
│   │   └── rpm/                 # RPM packages
│   ├── macos/                   # macOS packages (temporary staging)
│   │   ├── dmg/                 # DMG packages
│   │   └── pkg/                 # PKG packages
│   ├── source/                  # Source packages (temporary staging)
│   └── centralized/             # Final centralized release packages
│       └── v0.2.0/              # Version-specific centralized release
│           ├── *.deb            # DEB packages
│           ├── *.rpm            # RPM packages
│           ├── *.dmg            # DMG packages
│           ├── *.pkg            # PKG packages
│           ├── *-src.tar.gz     # Source tarballs
│           └── *-src.zip        # Source ZIP files
├── automation/ansible/
│   ├── scripts/
│   │   ├── remote-build.sh      # Remote build automation
│   │   ├── collect-packages.sh  # Package collection from VMs
│   │   └── organize-packages.sh # Package organization on VMs
│   └── playbook-build.yml       # Ansible playbook for builds
└── scripts/
    └── centralized-release.sh   # GitHub release automation
```

## Script Usage

### Remote Build Script

```bash
# Build on all remote VMs
cd automation/ansible
./scripts/remote-build.sh --packages

# Build on specific VM
ansible-playbook -i inventory-vmware.ini playbook-build.yml \
  --limit BUILD_MACOS --extra-vars "create_packages=true"
```

### Package Collection Script

```bash
# Collect packages from all remote VMs
cd automation/ansible
./scripts/collect-packages.sh
```

The script will:
- Check for packages in `dist/centralized/v{VERSION}/` on each VM (preferred)
- Fallback to `dist/` and `build/` directories
- Fetch all package types (DEB, RPM, DMG, PKG, source)
- Organize packages locally into `dist/centralized/v{VERSION}/`
- Remove hostname prefixes from package names

### Centralized Release Script

```bash
# Show help
./scripts/centralized-release.sh --help

# Scan for available packages
./scripts/centralized-release.sh --scan

# Create/update GitHub release
./scripts/centralized-release.sh --release
```

The script will:
- Find packages in `dist/centralized/v{VERSION}/`
- Create comprehensive release notes
- Create or update GitHub release
- Upload all packages as release assets
- Update Git tag to current HEAD if needed

## Example Workflow

### Complete Workflow (Recommended)

```bash
# 1. Build packages on all remote VMs
cd automation/ansible
./scripts/remote-build.sh --packages

# 2. Collect packages from all VMs
./scripts/collect-packages.sh

# 3. Create GitHub release
cd ../..
./scripts/centralized-release.sh --release
```

### Manual Workflow (if needed)

```bash
# 1. Build on specific VM
cd automation/ansible
ansible-playbook -i inventory-vmware.ini playbook-build.yml \
  --limit BUILD_MACOS --extra-vars "create_packages=true"

# 2. Collect packages
./scripts/collect-packages.sh

# 3. Verify packages
ls -la ../dist/centralized/v0.2.0/

# 4. Create release
cd ../..
./scripts/centralized-release.sh --release
```

## Prerequisites

### On Remote Build VMs

- simple-smtp-mailer source code
- Build dependencies installed
- Ansible connectivity from control machine
- `automation/ansible/scripts/organize-packages.sh` script (runs automatically)

### On Control Machine (Local)

- simple-smtp-mailer source code
- Ansible installed (`ansible-playbook` command)
- SSH access to remote VMs
- GitHub CLI (`gh`) installed and authenticated
- `automation/ansible/inventory-vmware.ini` configured

## GitHub CLI Setup

Install and authenticate GitHub CLI:

```bash
# Install (macOS)
brew install gh

# Install (Ubuntu)
sudo apt install gh

# Install (Windows)
winget install GitHub.cli

# Authenticate
gh auth login
```

## Troubleshooting

### No Packages Found

- Ensure packages were built successfully on remote VMs
- Check that `dist/centralized/v{VERSION}/` exists on remote VMs
- Verify `organize-packages.sh` ran successfully (check Ansible playbook output)
- Check fallback directories (`dist/` and `build/`) on remote VMs
- Verify package file extensions are supported

### GitHub Release Issues

- Check GitHub CLI authentication: `gh auth status`
- Ensure you have write access to the repository
- Check if release tag already exists

### Ansible Connection Issues

- Verify SSH connectivity to remote VMs: `ssh build@<vm-ip>`
- Check Ansible inventory file: `automation/ansible/inventory-vmware.ini`
- Test Ansible connection: `ansible all -i inventory-vmware.ini -m ping`
- Verify SSH keys are configured correctly
- Check Ansible inventory file syntax (no invalid `[hostname:vars]` sections)

## Benefits

1. **Centralized Management**: All releases managed from one location
2. **Cross-Platform Support**: Easy to include packages from multiple systems
3. **Consistent Process**: Same release process regardless of build platform
4. **Automated Workflow**: Reduces manual errors and saves time
5. **Comprehensive Documentation**: Automatic generation of release notes
6. **Version Control**: Easy to track what packages are included in each release

## Best Practices

1. **Always run remote build script** with `--packages` flag to build and organize packages
2. **Verify package collection** by checking `dist/centralized/v{VERSION}/` after collection
3. **Test the centralized release script** with `--scan` first to verify packages are found
4. **Keep centralized directories** for audit purposes
5. **Use consistent versioning** across all platforms (check Makefile VERSION)
6. **Update Git tag** if needed before creating release (script handles this automatically)
7. **Review release notes** before publishing (script creates draft releases)

## Support

If you encounter issues:

1. Check the script help: `./scripts/script-name.sh --help`
2. Review the generated log files and summaries
3. Verify all prerequisites are met
4. Check GitHub CLI status and permissions
5. Review the package collection structure
