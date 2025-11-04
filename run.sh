#!/bin/bash
set -e

PROJECT_DIR="$HOME/Documents/DSA_LAB"
BUILD_DIR="$PROJECT_DIR/build"
EXECUTABLE="$BUILD_DIR/BTreeVisualizer"

cd "$PROJECT_DIR"

if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

echo "🔧 Building project..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" >/dev/null
make -j"$(nproc)" >/dev/null

echo "🚀 Running B-Tree Visualizer..."
echo "---------------------------------"
"$EXECUTABLE"
