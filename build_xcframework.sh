#!/bin/bash

# プロジェクトとスキーム名
PROJECT_NAME="RealTimeCutVADCXXLibrary"
SCHEME_NAME="RealTimeCutVADCXXLibrary"
BUILD_DIR="./build"

# ビルド用ディレクトリをクリーン
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# ビルド関数
build_framework() {
    local sdk=$1
    local destination_dir=$2
    local release_dir=$3

    echo "Building for $sdk..."

    xcodebuild -project "${PROJECT_NAME}.xcodeproj" \
    -scheme "$SCHEME_NAME" \
    -sdk "$sdk" \
    -configuration Release \
    -derivedDataPath "$destination_dir" \
    BUILD_DIR="$destination_dir" \
    clean build

    FRAMEWORK_PATH="${destination_dir}/${release_dir}/${PROJECT_NAME}.framework"
    if [ ! -d "$FRAMEWORK_PATH" ]; then
        echo "Error: Failed to build $sdk framework."
        exit 1
    fi
}

# iOSデバイス向けビルド
build_framework "iphoneos" "$BUILD_DIR/ios_device" "Release-iphoneos"

# iOSシミュレータ向けビルド
build_framework "iphonesimulator" "$BUILD_DIR/ios_simulator" "Release-iphonesimulator"

# Mac向けビルド
build_framework "macosx" "$BUILD_DIR/macos" "Release"

# XCFrameworkの作成
echo "Creating XCFramework..."
xcodebuild -create-xcframework \
    -framework "$BUILD_DIR/ios_device/Release-iphoneos/${PROJECT_NAME}.framework" \
    -framework "$BUILD_DIR/ios_simulator/Release-iphonesimulator/${PROJECT_NAME}.framework" \
    -framework "$BUILD_DIR/macos/Release/${PROJECT_NAME}.framework" \
    -output "$BUILD_DIR/${PROJECT_NAME}.xcframework"

if [ -d "$BUILD_DIR/${PROJECT_NAME}.xcframework" ]; then
    echo "✅ XCFramework successfully created at $BUILD_DIR/${PROJECT_NAME}.xcframework"
else
    echo "❌ Failed to create XCFramework."
    exit 1
fi



