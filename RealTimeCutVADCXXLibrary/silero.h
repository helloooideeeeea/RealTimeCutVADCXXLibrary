//
// Created by Yasushi Sakita on 25/02/01.
//

#ifndef REALTIMECUTVAD_SILERO_H
#define REALTIMECUTVAD_SILERO_H


#include "realtime_cut_vad.h"
#include <onnxruntime/onnxruntime_cxx_api.h>

class RealTimeCutVAD::ISileroVAD {
public:
    virtual ~ISileroVAD() = default;
    virtual void reset_hidden_layer_value() = 0;
    virtual void init_onnx_model(const std::string& model_path) = 0;
    virtual float predict(const std::vector<float> &data) = 0;
};


class SileroV4 : public RealTimeCutVAD::ISileroVAD {
private:
    std::unique_ptr<Ort::Session> session;
    Ort::Env env;
    Ort::SessionOptions session_options;
    Ort::MemoryInfo memory_info;

    std::vector<const char *> input_node_names;
    int64_t sr;
    unsigned int size_hc;
    std::vector<float> _h;
    std::vector<float> _c;

    int64_t input_node_dims[2];
    const int64_t sr_node_dims[1];
    const int64_t hc_node_dims[3];

    std::vector<const char *> output_node_names;

public:
    SileroV4();
    ~SileroV4() override = default;

    void reset_hidden_layer_value() override;
    void init_onnx_model(const std::string& model_path) override;
    float predict(const std::vector<float>& data) override;
};

class SileroV5 : public RealTimeCutVAD::ISileroVAD{
private:
    std::unique_ptr<Ort::Session> session;
    Ort::Env env;
    Ort::SessionOptions session_options;
    Ort::MemoryInfo memory_info;

    std::vector<const char *> input_node_names;
    int64_t sr;
    unsigned int size_state;
    std::vector<float> _state;

    int64_t input_node_dims[2];
    const int64_t state_node_dims[3];
    const int64_t sr_node_dims[1];

    std::vector<const char *> output_node_names;

public:
    SileroV5();
    ~SileroV5() override = default;

    void reset_hidden_layer_value() override;
    void init_onnx_model(const std::string& model_path) override;
    float predict(const std::vector<float>& data) override;
};

#endif //REALTIMECUTVAD_SILERO_H
