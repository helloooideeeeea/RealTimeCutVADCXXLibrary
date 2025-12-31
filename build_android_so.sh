#!/bin/bash

set -e  # エラーが発生したら停止

NOW_DIR=`pwd`

if [ -z "$ANDROID_NDK" ]; then
    echo "ERROR: ANDROID_NDK が未設定です。例:"
    echo "  export ANDROID_NDK=\$HOME/Library/Android/sdk/ndk/26.1.10909125"
    exit 1
fi

TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"
if [ ! -f "$TOOLCHAIN_FILE" ]; then
    echo "ERROR: toolchain file が見つかりません: $TOOLCHAIN_FILE"
    exit 1
fi

GENERATOR="Unix Makefiles"
BUILD_CMD="make"

# ビルド関数
build_android() {
    ABI=$1
    OUTPUT_PATH=$NOW_DIR/jniLibs/$ABI

    echo "Building for $ABI..."

    # 以前のビルドディレクトリを削除して再作成
    rm -rf build_android
    mkdir -p build_android && cd build_android

    # CMake の設定とビルド
    cmake -G "$GENERATOR" \
          -DANDROID_ABI=$ABI \
          -DANDROID_NDK=$ANDROID_NDK \
          -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
          -DANDROID_PLATFORM=android-24 \
          -DCMAKE_BUILD_TYPE=Release \
          ../
    $BUILD_CMD -j

    mkdir -p $OUTPUT_PATH
    # 出力ディレクトリにコピー
    cp libRealtimeCutVadLibrary.so $OUTPUT_PATH

    echo "Build for $ABI completed. Output: $OUTPUT_PATH"

    # 元のディレクトリに戻る
    cd ..
}

# 各アーキテクチャごとにビルド
build_android arm64-v8a
build_android armeabi-v7a
build_android x86_64

# 元のディレクトリに戻る
cd ..

echo "All builds completed successfully!"