#!/bin/bash
# Usage: ./build.sh [gcc|clang] [Debug|Release]

# Ensure we're running from the project root
INSTALL_PREFIX="${PWD}/built"

# Set shared vcpkg installation directory
export VCPKG_INSTALLATION_ROOT="${PWD}/vcpkg_installed"
mkdir -p "$VCPKG_INSTALLATION_ROOT"

# Export the VCPKG manifest flag
export VCPKG_FEATURE_FLAGS=manifests

# Select configuration based on argument
if [ "$1" == "gcc" ]; then
  BUILD_DIR="build_gcc"
  COMPILER_C="gcc"
  COMPILER_CXX="g++"
elif [ "$1" == "clang" ]; then
  BUILD_DIR="build_clang"
  COMPILER_C="clang"
  COMPILER_CXX="clang++"
else
  echo "Usage: $0 [gcc|clang] [Debug|Release]"
  exit 1
fi

# Set build type
if [ -z "$2" ]; then
  BUILD_TYPE="Debug"
else
  BUILD_TYPE="$2"
fi

# Create the build and install directories if they don't exist
mkdir -p "$BUILD_DIR"
mkdir -p "$INSTALL_PREFIX"

# Resolve correct vcpkg toolchain path
VCPKG_TOOLCHAIN_FILE="$(realpath ${PWD}/../vcpkg/scripts/buildsystems/vcpkg.cmake)"

# Check if vcpkg toolchain file exists
if [ ! -f "$VCPKG_TOOLCHAIN_FILE" ]; then
  echo "Error: vcpkg toolchain file not found. Please check the path."
  exit 1
fi

# Configure the project
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN_FILE" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DCMAKE_C_COMPILER="$COMPILER_C" \
  -DCMAKE_CXX_COMPILER="$COMPILER_CXX"

# Build the project
cmake --build "$BUILD_DIR" --verbose

# Install the project
cmake --install "$BUILD_DIR"