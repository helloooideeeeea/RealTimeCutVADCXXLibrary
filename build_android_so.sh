#!/bin/bash

set -e  # エラーが発生したら停止

NOW_DIR=`pwd`

cd RealTimeCutVADCXXLibrary

# ビルド関数
build_android() {
    ABI=$1
    OUTPUT_PATH=$NOW_DIR/jniLibs/$ABI

    echo "Building for $ABI..."

    # 以前のビルドディレクトリを削除して再作成
    rm -rf build_android
    mkdir -p build_android && cd build_android

    # CMake の設定とビルド
    cmake -DANDROID_ABI=$ABI \
          -DANDROID_NDK=$ANDROID_NDK \
          -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
          -DANDROID_PLATFORM=android-24 \
          -DCMAKE_BUILD_TYPE=Release \
          ..

    make

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