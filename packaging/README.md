# Packaging Directory Structure

This directory contains the packaging templates and skeleton files for creating RPM and DEB packages for simple-smtp-mailer.

## Directory Structure

```
packaging/
├── linux/
│   ├── deb/                    # Debian/Ubuntu package files
│   │   ├── control             # DEB package control file
│   │   ├── postinst            # Post-installation script
│   │   ├── postrm              # Post-removal script
│   │   └── prerm               # Pre-removal script
│   └── rpm/                     # Red Hat/CentOS/Fedora package files
│       ├── simple-smtp-mailer.spec  # RPM spec file
│       ├── preinstall.sh       # Pre-installation script
│       ├── postinstall.sh      # Post-installation script
│       ├── preuninstall.sh     # Pre-uninstallation script
│       └── postuninstall.sh    # Post-uninstallation script
├── macos/                      # macOS packaging files
└── windows/                    # Windows packaging files
```

## Package Contents

### RPM Package Structure

The RPM package (`simple-smtp-mailer.spec`) installs:

- **Binary**: `/usr/bin/simple-smtp-mailer`
- **Configuration**: `/etc/simple-smtp-mailer/simple-smtp-mailer.conf.example`
- **Configuration snippets**: `/etc/simple-smtp-mailer/conf.d/*.example`
- **Systemd service**: `/usr/lib/systemd/system/simple-smtp-mailer.service`
- **OAuth2 helper tools**: `/usr/share/simple-smtp-mailer/oauth2-helper/`
- **Documentation**: `/usr/share/doc/simple-smtp-mailer/`

**Dependencies**:
- `openssl-libs >= 3.0`
- `jsoncpp`
- `libcurl`
- `python3-requests`

### DEB Package Structure

The DEB package (`control`) installs:

- **Binary**: `/usr/bin/simple-smtp-mailer`
- **Configuration**: `/etc/simple-smtp-mailer/simple-smtp-mailer.conf.example`
- **Configuration snippets**: `/etc/simple-smtp-mailer/conf.d/*.example`
- **Systemd service**: `/lib/systemd/system/simple-smtp-mailer.service`
- **OAuth2 helper tools**: `/usr/share/simple-smtp-mailer/oauth2-helper/`
- **Documentation**: `/usr/share/doc/simple-smtp-mailer/`

**Dependencies**:
- `libssl3 (>= 3.0.0)`
- `libjsoncpp25 (>= 1.9.0)`
- `libcurl4 (>= 7.68.0)`
- `libc6 (>= 2.31)`
- `python3`
- `python3-requests`

## Building Packages

### Using CMake/CPack (Recommended)

```bash
# Build the project
make build

# Create DEB package
make package-deb

# Create RPM package
make package-rpm
```

### Manual RPM Build

```bash
# Install build dependencies
sudo yum install rpm-build rpmdevtools

# Build RPM
rpmbuild -ba packaging/linux/rpm/simple-smtp-mailer.spec
```

### Manual DEB Build

```bash
# Install build dependencies
sudo apt-get install dpkg-dev debhelper build-essential

# Build DEB (requires proper Debian package structure)
dpkg-buildpackage -us -uc
```

## Package File Locations

After building, packages are located in:

- **RPM**: `dist/simple-smtp-mailer-0.2.0-Linux.rpm`
- **DEB**: `dist/simple-smtp-mailer-0.2.0-Linux.deb` (or similar naming)

## Installation

### Install RPM Package

```bash
sudo rpm -ivh dist/simple-smtp-mailer-0.2.0-Linux.rpm
```

### Install DEB Package

```bash
sudo dpkg -i dist/simple-smtp-mailer-0.2.0-Linux.deb
```

## Troubleshooting

### DEB Package Not Created

If the DEB package is not being created:

1. **Check for required tools**:
   ```bash
   which dpkg-deb
   # If missing: sudo apt-get install dpkg-dev
   ```

2. **Check CPack output**:
   ```bash
   cd build
   cpack -G DEB -V
   ```

3. **Check file permissions**:
   ```bash
   ls -la build/*.deb
   ```

4. **Verify CMake configuration**:
   ```bash
   grep ENABLE_PACKAGING build/CMakeCache.txt
   # Should show: ENABLE_PACKAGING:BOOL=ON
   ```

### RPM Package Not Created

If the RPM package is not being created:

1. **Check for required tools**:
   ```bash
   which rpmbuild
   # If missing: sudo yum install rpm-build
   ```

2. **Check CPack output**:
   ```bash
   cd build
   cpack -G RPM -V
   ```

## Package Metadata

### Version Information

Update version in:
- `CMakeLists.txt`: `project(simple-smtp-mailer VERSION 0.2.0 ...)`
- `packaging/linux/rpm/simple-smtp-mailer.spec`: `Version: 0.2.0`
- `packaging/linux/deb/control`: `Version: 0.2.0`

### Maintainer Information

Update maintainer in:
- `CMakeLists.txt`: `CPACK_PACKAGE_VENDOR` and `CPACK_PACKAGE_CONTACT`
- `packaging/linux/rpm/simple-smtp-mailer.spec`: `%changelog` section
- `packaging/linux/deb/control`: `Maintainer:` field

## Notes

- All scripts must be executable (`chmod +x`)
- Package files are templates - actual packages are built by CPack
- The spec file and control file are reference implementations
- CPack uses CMakeLists.txt settings for actual package creation
