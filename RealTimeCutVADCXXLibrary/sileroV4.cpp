//
// Created by Yasushi Sakita on 25/02/01.
//

#include "silero.h"

SileroV4::SileroV4()
        : session(nullptr),
          env(ORT_LOGGING_LEVEL_WARNING, "SileroV4"),
          session_options(),
          memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU)),
          input_node_names({"input", "sr", "h", "c"}),
          sr(16000),
          size_hc(2 * 1 * 64),
          _h(size_hc, 0.0f),
          _c(size_hc, 0.0f),
          input_node_dims{},
          sr_node_dims{1},
          hc_node_dims{2, 1, 64},
          output_node_names({"output", "hn", "cn"}){
}

void SileroV4::reset_hidden_layer_value() {
    _h.assign(size_hc, 0.0f);
    _c.assign(size_hc, 0.0f);
}

void SileroV4::init_onnx_model(const std::string& model_path) {
    session.reset();
    reset_hidden_layer_value();

    session_options.SetIntraOpNumThreads(1);
    session_options.SetInterOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    try {
        session = std::make_unique<Ort::Session>(env, model_path.c_str(), session_options);
    } catch (const Ort::Exception& e) {
        std::cerr << "Failed to create ONNX session: " << e.what() << std::endl;
        throw;
    }
}

float SileroV4::predict(const std::vector<float>& data) {
    try {
        // Assume input dimensions are [batch, feature] and batch is 1 for simplicity
        input_node_dims[0] = 1; // Batch size
        input_node_dims[1] = data.size(); // Feature size

        // TODO あとでテスト
        float* buffer = const_cast<float*>(data.data());
        // Prepare input tensors
        Ort::Value input_ort = Ort::Value::CreateTensor<float>(
                memory_info, buffer, data.size(), input_node_dims, 2);

        // Prepare other inputs and run inference
        Ort::Value sr_ort = Ort::Value::CreateTensor<int64_t>(
                memory_info, &sr, 1, sr_node_dims, 1);

        Ort::Value h_ort = Ort::Value::CreateTensor<float>(
                memory_info, _h.data(), _h.size(), hc_node_dims, 3);

        Ort::Value c_ort = Ort::Value::CreateTensor<float>(
                memory_info, _c.data(), _c.size(), hc_node_dims, 3);

        std::vector<Ort::Value> ort_inputs;

        // Move Ort::Value objects into the vector
        ort_inputs.emplace_back(std::move(input_ort));
        ort_inputs.emplace_back(std::move(sr_ort));
        ort_inputs.emplace_back(std::move(h_ort));
        ort_inputs.emplace_back(std::move(c_ort));

        // Run model
        auto ort_outputs = session->Run(
                Ort::RunOptions{nullptr},
                input_node_names.data(),
                ort_inputs.data(), // Pass the address of the data in the vector
                ort_inputs.size(), // Correct number of inputs
                output_node_names.data(),
                output_node_names.size()
        );

        // Process outputs
        float speech_prob = ort_outputs[0].GetTensorMutableData<float>()[0];
        float *hn = ort_outputs[1].GetTensorMutableData<float>();
        std::memcpy(_h.data(), hn, size_hc * sizeof(float));
        float *cn = ort_outputs[2].GetTensorMutableData<float>();
        std::memcpy(_c.data(), cn, size_hc * sizeof(float));
        return speech_prob;

    } catch (const Ort::Exception& e) {
        std::cerr << "Prediction failed: " << e.what() << std::endl;
        return 0.0f; // Handle error appropriately
    }
}