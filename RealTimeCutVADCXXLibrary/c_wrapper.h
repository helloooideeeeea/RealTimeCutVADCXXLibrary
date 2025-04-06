//
//  c_wrapper.h
//  RealTimeCutVADCXXLibrary
//
//  Created by Yasushi Sakita on 2025/02/06.
//

#ifndef c_wrapper_h
#define c_wrapper_h

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* VADInstanceHandle;

typedef void (*VoiceStartCallback)(void* context);
typedef void (*VoiceEndCallback)(void* context, const uint8_t* wav_data, size_t wav_size);
typedef void (*VoiceDidContinueCallback)(void* context, const uint8_t* pcm_float_data, size_t data_size);

// インスタンスの作成と破棄
VADInstanceHandle create_vad_instance();
void destroy_vad_instance(VADInstanceHandle instance);

// コールバックの設定
void set_vad_callback(VADInstanceHandle instance,
                      void* context,
                      VoiceStartCallback start_cb,
                      VoiceEndCallback end_cb,
                      VoiceDidContinueCallback continue_cb);

// パラメータ設定
void set_vad_sample_rate(VADInstanceHandle instance, int sample_rate);
void set_vad_threshold(VADInstanceHandle instance, float vad_start_prob,
                       float vad_end_prob, float start_true_ratio,
                       float end_false_ratio, int start_frame_count, int end_frame_count);

// モデルの設定
void set_vad_model(VADInstanceHandle instance, int model_version, const char* model_path);

// オーディオ処理
void process_vad_audio(VADInstanceHandle instance, const float* audio_data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* c_wrapper_h */
