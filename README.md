# RealTimeCutVADCXXLibrary

**RealTimeCutVADCXXLibrary** is the core C++ implementation of a real-time Voice Activity Detection (VAD) system utilizing **ONNX Runtime** for running Silero VAD models and **WebRTC's Audio Processing Module (APM)** for advanced audio preprocessing. This library is responsible for the detailed audio segmentation and detection algorithms. It is used in **RealTimeCutVADLibrary**.

---

## Features

- **Real-time Voice Activity Detection (VAD)** using Silero models (v4 or v5.. recommend v5)
- **High-performance inference** with ONNX Runtime (C++)
- **Advanced audio preprocessing** using WebRTC's APM (noise suppression, gain control, high-pass filtering)
- **Accurate voice segmentation algorithms** implemented entirely in C++
- **Real-time WAV generation with callback support** for seamless integration
- **Cross-platform compatibility** through XCFramework and Android support

---

## Building for iOS/macOS (XCFramework)

**RealTimeCutVADCXXLibrary** is the core C++ implementation, designed to be compiled into an XCFramework. This project is not intended for direct integration via Swift Package Manager (SPM) or CocoaPods.

### How to Build

You can generate the XCFramework by running the provided build script. This script compiles the library for iOS devices, iOS simulators, and macOS, and then packages them into a single XCFramework.

### Steps to Build:

1. Clone the repository:

```bash
git clone https://github.com/helloooideeeeea/RealTimeCutVADCXXLibrary.git
cd RealTimeCutVADCXXLibrary
```

2. Download and extract the required XCFrameworks into the `Frameworks/` directory:

```bash
wget https://github.com/helloooideeeeea/RealTimeCutVADLibraryForXCFramework/releases/download/v1.0.0/onnxruntime.xcframework.zip
wget https://github.com/helloooideeeeea/RealTimeCutVADLibraryForXCFramework/releases/download/v1.0.0/webrtc_audio_processing.xcframework.zip
unzip onnxruntime.xcframework.zip -d Frameworks/
unzip webrtc_audio_processing.xcframework.zip -d Frameworks/
```

3. Run the build script:

```bash
./build_xcframework.sh
```

### Script Details

The `build_xcframework.sh` script will:
- Clean any existing build artifacts.
- Build the library for:
    - iOS devices (`iphoneos`)
    - iOS simulators (`iphonesimulator`)
    - macOS (`macosx`)
- Create the XCFramework combining all platforms.

### Output

Upon successful execution, the XCFramework will be generated at:

```bash
./build/RealTimeCutVADCXXLibrary.xcframework
```

You can then integrate this XCFramework into your iOS or macOS projects.

---

## Building for Android

### How to Build

**RealTimeCutVADCXXLibrary** now supports building for Android! You can compile the shared library (`.so`) for different Android architectures using the provided script.

### Steps to Build:

1. Ensure that the `ANDROID_NDK` environment variable is correctly set. You can set it externally before running the script:

```bash
export ANDROID_NDK=$ANDROID_HOME/ndk/25.1.8937393
```

2. Download and extract the prebuilt Android libraries (`jniLibs.zip`) into the `Project Root` directory:

```bash
wget https://github.com/helloooideeeeea/RealTimeCutVADCXXLibrary/releases/download/v1.0.2/RealTimeCutVADCXXLibrary.jniLibs.zip
unzip RealTimeCutVADCXXLibrary.jniLibs.zip -d ./
```

3. Run the Android build script:

```bash
./build_android_so.sh
```

### Script Details

The `build_android_so.sh` script will:
- Clean any existing build artifacts.
- Build the library for:
    - `arm64-v8a`
    - `armeabi-v7a`
    - `x86_64`
- Copy the generated `.so` files to `jniLibs/` for Android integration.

### Output

Upon successful execution, the shared libraries will be generated at:

```bash
jniLibs/
├── arm64-v8a/
│   ├── libRealtimeCutVadLibrary.so
├── armeabi-v7a/
│   ├── libRealtimeCutVadLibrary.so
├── x86_64/
│   ├── libRealtimeCutVadLibrary.so
```

You can now use these libraries in your Android project under `src/main/jniLibs/`.

---

## Using the Library in C++

Here's how you can utilize **RealTimeCutVADCXXLibrary** directly in your C++ projects for real-time voice activity detection and WAV file generation.

### Example Code

```cpp
#include <iostream>
#include <vector>
#include <sndfile.h>
#include <thread>
#include <fstream>
#include <thread>
#include <filesystem>
#include <memory>
#include "realtime_cut_vad.h"

std::string time_str() {
    auto now = std::chrono::system_clock::now(); // Get current time
    auto now_as_time_t = std::chrono::system_clock::to_time_t(now); // Convert to time_t
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    long duration = value.count() % 1000; // Extract milliseconds

    std::tm tm = *std::localtime(&now_as_time_t); // Convert to std::tm
    char buffer[32]; // Sufficient buffer size
    std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &tm); // Format: YYYYMMDDHHMMSS

    std::ostringstream oss;
    oss << buffer << std::setw(3) << std::setfill('0') << duration; // Append milliseconds
    return oss.str();
}

void voiceStartCallback(void* context)
{
    std::cout << "Voice recording started." << std::endl;
}

void voiceEndCallback(void* context, const uint8_t* wav_data, size_t wav_size)
{
    std::cout << "Recording finished." << std::endl;
    std::filesystem::path filename = std::filesystem::path(PROJECT_SOURCE_DIR) / ("raw_" + time_str() + ".wav");

    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Failed to open file for writing: " << filename << std::endl;
        return;
    }

    outfile.write(reinterpret_cast<const char*>(wav_data), wav_size);
    outfile.close();

    std::cout << "Partial recorded audio file saved: " << filename << std::endl;
}

int main(int argc, char *argv[]) {
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <onnx model file>" << " <Silero Version v4 or v5>" << " <Sample Rate>" << " <wav file>" << std::endl;
        return 1;
    }

    std::string model_path = argv[1];
    std::string model_version = argv[2];
    int sample_rate = std::atoi(argv[3]);
    char* wave_path = argv[4];

    SF_INFO sfinfo;
    SNDFILE *infile = sf_open(wave_path, SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Failed to open file: " << wave_path << std::endl;
        return 1;
    }

    std::vector<float> input(sfinfo.frames * sfinfo.channels);
    sf_count_t num_items = sfinfo.frames * sfinfo.channels;

    sf_count_t num_read = sf_read_float(infile, input.data(), num_items);
    if (num_read != num_items) {
        std::cerr << "Failed to read all frames" << std::endl;
        sf_close(infile);
        return 1;
    }

    std::cout << "floats num " << input.size() << std::endl;

    std::unique_ptr<RealTimeCutVAD> vad = std::make_unique<RealTimeCutVAD>();

    if (model_version == "v4") {
        vad->setModel(V4, std::string(model_path));
    } else if (model_version == "v5") {
        vad->setModel(V5, std::string(model_path));
    } else {
        std::cerr << "Invalid model version: " << model_version << std::endl;
        return 1;
    }

    switch (sample_rate) {
        case 8000:
            vad->setSampleRate(K_8);
            break;
        case 16000:
            vad->setSampleRate(K_16);
            break;
        case 24000:
            vad->setSampleRate(K_24);
            break;
        case 48000:
            vad->setSampleRate(K_48);
            break;
        default:
            std::cerr << "Invalid sample rate: " << sample_rate << std::endl;
            return 1;
    }

    vad->setCallback(
            NULL,
            voiceStartCallback,
            voiceEndCallback
    );

    vad->process(input);

    sf_close(infile);
    return 0;
}
```

### Explanation
- **Initialization**: The library initializes the VAD model and sets up the audio processing environment.
- **Callbacks**: The `voiceStartCallback` and `voiceEndCallback` handle the start and end of detected voice activity.
- **Real-Time Processing**: The input WAV file is processed in real-time, and detected voice segments are saved as separate WAV files.

---

## Algorithm Explanation

### ONNX Runtime for Silero VAD
This library leverages **ONNX Runtime (C++)** to run Silero VAD models efficiently, ensuring fast and accurate voice activity detection.

### WebRTC's Audio Processing Module (APM)
WebRTC's APM is used for:

- **High-pass Filtering**: Removes low-frequency noise.
- **Noise Suppression**: Reduces background noise for clearer detection.
- **Gain Control**: Adaptive digital gain control enhances audio levels.
- **Sample Rate Conversion**: Converts audio to 16 kHz for Silero VAD compatibility.

### Audio Processing Workflow

1. **Input Audio Configuration**: Supports sample rates of 8 kHz, 16 kHz, 24 kHz, and 48 kHz.
2. **Audio Preprocessing**:
    - Audio is split into chunks and processed with APM.
    - Audio is converted to 16 kHz for compatibility with Silero VAD.
3. **Voice Activity Detection**:
    - Processed audio is passed to Silero VAD via ONNX Runtime.
    - VAD outputs a probability score indicating voice activity.
4. **Voice Segmentation Algorithm**:
    - **Voice Start**: Triggered when the probability exceeds the start threshold for a set number of frames.
    - **Voice End**: Triggered when silence is detected over a set number of frames.
5. **Output**: The final WAV data is generated, including proper header information for immediate playback or saving.

### WebRTC APM Configuration

```cpp
config.gain_controller1.enabled = true;
config.gain_controller1.mode = webrtc::AudioProcessing::Config::GainController1::kAdaptiveDigital;
config.gain_controller2.enabled = true;
config.high_pass_filter.enabled = true;
config.noise_suppression.enabled = true;
config.transient_suppression.enabled = true;
config.voice_detection.enabled = false;
```

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.



