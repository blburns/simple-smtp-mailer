# CMakeLists.txt for simple-smtp-mailer
# High-performance SMTP mailer with queue management and OAuth2 support
# Copyright 2024 blburns

cmake_minimum_required(VERSION 3.16)
project(simple-smtp-mailer VERSION 0.2.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "Windows")
    set(OS_TYPE "win")
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
    set(OS_TYPE "macos")
    # Detect macOS architecture
    # Check if building universal binary (both x86_64 and arm64)
    if(BUILD_UNIVERSAL_BINARY)
        # Force universal binary if option is set
        if(NOT CMAKE_OSX_ARCHITECTURES)
            set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
        endif()
        set(ARCH "universal")
        message(STATUS "Building universal binary (Intel + Apple Silicon)")
    elseif(CMAKE_OSX_ARCHITECTURES)
        # Check if CMAKE_OSX_ARCHITECTURES contains both architectures
        string(FIND "${CMAKE_OSX_ARCHITECTURES}" "x86_64" HAS_X86_64)
        string(FIND "${CMAKE_OSX_ARCHITECTURES}" "arm64" HAS_ARM64)
        if(NOT HAS_X86_64 EQUAL -1 AND NOT HAS_ARM64 EQUAL -1)
            # Universal binary (both architectures)
            set(ARCH "universal")
            message(STATUS "Detected universal binary build (Intel + Apple Silicon)")
        elseif(NOT HAS_X86_64 EQUAL -1)
            # Intel only
            set(ARCH "intel")
        elseif(NOT HAS_ARM64 EQUAL -1)
            # Apple Silicon only
            set(ARCH "aarch64")
        else()
            # Fallback to system processor
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i386")
                set(ARCH "intel")
            elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
                set(ARCH "aarch64")
            else()
                set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
            endif()
        endif()
    else()
        # No CMAKE_OSX_ARCHITECTURES set, detect from system
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i386")
            set(ARCH "intel")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            set(ARCH "aarch64")
        else()
            set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    endif()
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
    set(OS_TYPE "linux")
    
    # Detect Linux distribution
    if(EXISTS "/etc/os-release")
        file(READ "/etc/os-release" OS_RELEASE)
        if(OS_RELEASE MATCHES "ID=debian|ID=ubuntu")
            set(DISTRIBUTION "debian")
        elseif(OS_RELEASE MATCHES "ID=fedora|ID=rhel|ID=centos|ID=rocky|ID=almalinux")
            set(DISTRIBUTION "redhat")
        elseif(OS_RELEASE MATCHES "ID=arch")
            set(DISTRIBUTION "arch")
        elseif(OS_RELEASE MATCHES "ID=suse|ID=opensuse")
            set(DISTRIBUTION "suse")
        else()
            set(DISTRIBUTION "generic")
        endif()
    elseif(EXISTS "/etc/redhat-release")
        set(DISTRIBUTION "redhat")
    elseif(EXISTS "/etc/debian_version")
        set(DISTRIBUTION "debian")
    else()
        set(DISTRIBUTION "generic")
    endif()
    
    # Detect Linux architecture (use "amd64" for x86_64)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(ARCH "amd64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(ARCH "aarch64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l|armv7")
        set(ARCH "armv7")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686|x86")
        set(ARCH "x86")
    else()
        set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
else()
    set(OS_TYPE "unknown")
    set(DISTRIBUTION "generic")
    # Default architecture detection for unknown platforms
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(ARCH "amd64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(ARCH "aarch64")
    else()
        set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_PACKAGING "Enable package generation" ON)
option(ENABLE_SSL "Enable SSL/TLS support" ON)
option(ENABLE_JSON "Enable JSON support" ON)
option(ENABLE_CURL "Enable libcurl support" ON)
option(ENABLE_STATIC_LINKING "Enable static linking for self-contained binaries" OFF)
option(BUILD_UNIVERSAL_BINARY "Build universal binary for macOS (Intel + Apple Silicon)" OFF)

# Find required packages
find_package(Threads REQUIRED)

if(ENABLE_SSL)
        find_package(OpenSSL REQUIRED)
endif()

if(ENABLE_JSON)
find_package(PkgConfig REQUIRED)
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)
endif()

if(ENABLE_CURL)
find_package(CURL REQUIRED)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h")

# Filter out test files from main sources
list(FILTER SOURCES EXCLUDE REGEX ".*test.*\\.cpp$")

# Create library first
add_library(${PROJECT_NAME}-lib ${SOURCES} ${HEADERS})

# Create executable that links to the library
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-lib)

# Link libraries
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME}-lib Threads::Threads)

if(ENABLE_SSL)
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(${PROJECT_NAME}-lib OpenSSL::SSL OpenSSL::Crypto)
endif()

if(ENABLE_JSON)
    if(ENABLE_STATIC_LINKING)
        # For static linking, find the static library directly
        find_library(JSONCPP_STATIC_LIB jsoncpp PATHS /opt/local/lib /usr/local/lib /usr/lib)
        if(JSONCPP_STATIC_LIB)
            target_link_libraries(${PROJECT_NAME} ${JSONCPP_STATIC_LIB})
            target_link_libraries(${PROJECT_NAME}-lib ${JSONCPP_STATIC_LIB})
        else()
            target_link_libraries(${PROJECT_NAME} ${JSONCPP_LIBRARIES})
            target_link_libraries(${PROJECT_NAME}-lib ${JSONCPP_LIBRARIES})
        endif()
    else()
        target_link_libraries(${PROJECT_NAME} ${JSONCPP_LIBRARIES})
        target_link_libraries(${PROJECT_NAME}-lib ${JSONCPP_LIBRARIES})
    endif()
    target_include_directories(${PROJECT_NAME} PRIVATE ${JSONCPP_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${JSONCPP_CFLAGS_OTHER})
    target_link_directories(${PROJECT_NAME} PRIVATE ${JSONCPP_LIBRARY_DIRS})

    # Also add JSON include directories to the library
    target_include_directories(${PROJECT_NAME}-lib PRIVATE ${JSONCPP_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME}-lib PRIVATE ${JSONCPP_CFLAGS_OTHER})
    target_link_directories(${PROJECT_NAME}-lib PRIVATE ${JSONCPP_LIBRARY_DIRS})
endif()

if(ENABLE_CURL)
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}-lib ${CURL_LIBRARIES})
    target_include_directories(${PROJECT_NAME}-lib PRIVATE ${CURL_INCLUDE_DIRS})
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
    if(ENABLE_STATIC_LINKING)
        target_compile_options(${PROJECT_NAME} PRIVATE /MT)
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-private-field)
    if(ENABLE_STATIC_LINKING)
        # Only apply static linking flags for GCC, not clang
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
            target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
        endif()
    endif()
endif()

# Static linking configuration
if(ENABLE_STATIC_LINKING)
    set(BUILD_SHARED_LIBS OFF)
    if(ENABLE_SSL)
        # Force static linking for OpenSSL
        set(OPENSSL_USE_STATIC_LIBS TRUE)
    endif()
    if(ENABLE_JSON)
        # Force static linking for jsoncpp
        set(JSONCPP_USE_STATIC_LIBS TRUE)
    endif()
    if(ENABLE_CURL)
        # Force static linking for libcurl
        set(CURL_USE_STATIC_LIBS TRUE)
    endif()
endif()

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install configuration files
install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.conf" PATTERN "*.example"
)

# Install OAuth2 helper tools
install(DIRECTORY tools/oauth2-helper/
    DESTINATION share/${PROJECT_NAME}/oauth2-helper
)

# Install documentation
install(DIRECTORY docs/
    DESTINATION share/${PROJECT_NAME}/docs
)

# Install service files
if(UNIX AND NOT APPLE)
    install(FILES deployment/systemd/${PROJECT_NAME}.service
        DESTINATION lib/systemd/system
    )
elseif(APPLE)
    install(FILES deployment/launchd/com.${PROJECT_NAME}.${PROJECT_NAME}.plist
        DESTINATION /Library/LaunchDaemons
    )
endif()

# Tests
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Package generation
if(ENABLE_PACKAGING)
    # Package information (must be set BEFORE include(CPack))
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR "blburns")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-performance SMTP mailer with queue management and OAuth2 support")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_CONTACT "support@blburns.com")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/blburns/simple-smtp-mailer")

    # Package files
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    # Platform-specific package settings
    if(WIN32)
        # Windows MSI and ZIP packages
        set(CPACK_GENERATOR "WIX;ZIP")

        # WIX (MSI) settings
        set(CPACK_WIX_PRODUCT_GUID "12345678-1234-1234-1234-123456789012")
        set(CPACK_WIX_UPGRADE_GUID "87654321-4321-4321-4321-210987654321")
        set(CPACK_WIX_PROPERTY_ARPPRODUCTICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico")
        set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://github.com/blburns/simple-smtp-mailer")
        set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/blburns/simple-smtp-mailer")
        set(CPACK_WIX_PROPERTY_ARPNOREPAIR "1")
        set(CPACK_WIX_PROPERTY_ARPNOMODIFY "1")

        # ZIP package settings
        set(CPACK_ZIP_COMPONENT_INSTALL ON)

    elseif(APPLE)
        # macOS DMG and PKG packages
        set(CPACK_GENERATOR "DragNDrop;productbuild")

        # DMG settings
        set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
        set(CPACK_DMG_FORMAT "UDBZ")
        set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/assets/dmg_background.png")
        set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/assets/DS_Store")

        # PKG settings
        set(CPACK_PRODUCTBUILD_IDENTIFIER "com.blburns.${PROJECT_NAME}")
        set(CPACK_PRODUCTBUILD_PACKAGE_NAME "${PROJECT_NAME}")

    else()
        # Linux DEB and RPM packages
        set(CPACK_GENERATOR "DEB;RPM")

        # DEB package settings
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "blburns <support@blburns.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "mail")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libssl3, libjsoncpp25, libcurl4")
        set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "systemd")
        set(CPACK_DEBIAN_PACKAGE_SUGGESTS "logrotate")
        set(CPACK_DEBIAN_PACKAGE_CONFLICTS "postfix, sendmail")
        set(CPACK_DEBIAN_PACKAGE_PROVIDES "${PROJECT_NAME}")
        set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/blburns/simple-smtp-mailer")
        set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "High-performance SMTP mailer with queue management and OAuth2 support")
        set(CPACK_DEBIAN_PACKAGE_LONG_DESCRIPTION "A modern, high-performance SMTP mailer with advanced features including queue management, OAuth2 authentication, and multi-provider API support.")

        # RPM package settings
        set(CPACK_RPM_PACKAGE_LICENSE "Apache-2.0")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
        set(CPACK_RPM_PACKAGE_URL "https://github.com/blburns/simple-smtp-mailer")
        set(CPACK_RPM_PACKAGE_DESCRIPTION "High-performance SMTP mailer with queue management and OAuth2 support")
        set(CPACK_RPM_PACKAGE_REQUIRES "openssl-libs, jsoncpp, libcurl")
        set(CPACK_RPM_PACKAGE_SUGGESTS "systemd")
        set(CPACK_RPM_PACKAGE_CONFLICTS "postfix, sendmail")
        set(CPACK_RPM_PACKAGE_PROVIDES "${PROJECT_NAME}")

        # Set package architecture
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
            set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
            set(CPACK_RPM_PACKAGE_ARCHITECTURE "aarch64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
            set(CPACK_RPM_PACKAGE_ARCHITECTURE "armv7hl")
        endif()
        
    endif()

    # Common package settings
    # Use standardized naming: {app_name}-{version}-{os_type}-{distribution}-{arch}
    # CPack will add the appropriate extension (.deb, .rpm, .tar.gz, .dmg, .pkg, etc.)
    if(WIN32)
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${OS_TYPE}-${ARCH}")
    elseif(APPLE)
        # macOS: {app_name}-{version}-{os_type}-{arch}
        # Examples: simple-smtp-mailer-0.2.0-macos-intel.dmg
        #          simple-smtp-mailer-0.2.0-macos-aarch64.dmg
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${OS_TYPE}-${ARCH}")
    else()
        # Linux uses distribution-specific naming
        # Format: {app_name}-{version}-{os_type}-{distribution}-{arch}
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${OS_TYPE}-${DISTRIBUTION}-${ARCH}")
    endif()
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")

    # Component installation
    set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Application")
    set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "The ${PROJECT_NAME} executable and related files")

    set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "Development")
    set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "Header files for developing with ${PROJECT_NAME}")

    set(CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Configuration")
    set(CPACK_COMPONENT_CONFIG_DESCRIPTION "Configuration files and examples")

    set(CPACK_COMPONENT_SERVICE_DISPLAY_NAME "Service")
    set(CPACK_COMPONENT_SERVICE_DESCRIPTION "System service files")

    set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
    set(CPACK_COMPONENT_TOOLS_DESCRIPTION "OAuth2 helper tools and utilities")

    set(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation")
    set(CPACK_COMPONENT_DOCS_DESCRIPTION "Documentation and examples")

    # Set component dependencies
    set(CPACK_COMPONENT_DEVELOPMENT_DEPENDS applications)
    set(CPACK_COMPONENT_CONFIG_DEPENDS applications)
    set(CPACK_COMPONENT_SERVICE_DEPENDS applications)
    set(CPACK_COMPONENT_TOOLS_DEPENDS applications)
    set(CPACK_COMPONENT_DOCS_DEPENDS applications)

    # Source package generation
    set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-src")
    set(CPACK_SOURCE_IGNORE_FILES
        "/build/"
        "/dist/"
        "/.git/"
        "/.gitignore"
        "/.vscode/"
        "/.vs/"
        "/CMakeCache.txt"
        "/CMakeFiles/"
        "/cmake_install.cmake"
        "/Makefile"
        "/install_manifest.txt"
        "/compile_commands.json"
        "/CTestTestfile.cmake"
        "/_CPack_Packages/"
        "/.DS_Store"
        "/Thumbs.db"
        "*.o"
        "*.a"
        "*.so"
        "*.dylib"
        "*.dll"
        "*.exe"
        "*.pdb"
        "*.ilk"
        "*.exp"
        "*.lib"
    )

    # Include CPack AFTER all CPACK variables are set
    # CPack reads variables when include() is called, so they must be set first
    include(CPack)

endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Shared libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "  Tests enabled: ${ENABLE_TESTS}")
message(STATUS "  Packaging enabled: ${ENABLE_PACKAGING}")
message(STATUS "  SSL support: ${ENABLE_SSL}")
message(STATUS "  JSON support: ${ENABLE_JSON}")
message(STATUS "  CURL support: ${ENABLE_CURL}")
message(STATUS "  Static linking: ${ENABLE_STATIC_LINKING}")
message(STATUS "")
