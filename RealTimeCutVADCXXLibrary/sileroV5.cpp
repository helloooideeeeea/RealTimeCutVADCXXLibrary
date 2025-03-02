//
// Created by Yasushi Sakita on 25/02/01.
//

#include "silero.h"

SileroV5::SileroV5()
        : session(nullptr),
          env(ORT_LOGGING_LEVEL_WARNING, "SileroV5"),
          session_options(),
          memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU)),
          input_node_names({"input", "state", "sr"}),
          sr(16000),
          size_state(2 * 1 * 128),
          _state(size_state, 0.0f),
          state_node_dims{2, 1, 128},
          sr_node_dims{1},
          context_size(64),
          _context(context_size, 0.0f),
          output_node_names({"output", "stateN"}){
}

void SileroV5::reset_hidden_layer_value() {
    _state.assign(size_state, 0.0f);
    _context.assign(context_size, 0.0f);
}

void SileroV5::init_onnx_model(const std::string& model_path) {
    // Clear previous model session if any
    session.reset();

    // Initialize internal state vectors
    reset_hidden_layer_value();

    // Configure session options
    session_options.SetIntraOpNumThreads(1);
    session_options.SetInterOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    // Create a new session
    try {
        session = std::make_unique<Ort::Session>(env, model_path.c_str(), session_options);
    } catch (const Ort::Exception& e) {
        std::cerr << "Failed to create ONNX session: " << e.what() << std::endl;
        throw;
    }
}

float SileroV5::predict(const std::vector<float> &data) {
    try {

        // コンテキストと入力データを連結する
        std::vector<float> input_with_context;
        input_with_context.reserve(_context.size() + data.size());
        input_with_context.insert(input_with_context.end(), _context.begin(), _context.end());
        input_with_context.insert(input_with_context.end(), data.begin(), data.end());

        // 連結した入力の次元は [1, context_size + data.size()]
        int64_t input_dims[2] = {1, static_cast<int64_t>(input_with_context.size())};

        Ort::Value input_ort = Ort::Value::CreateTensor<float>(
            memory_info, input_with_context.data(), input_with_context.size(), input_dims, 2);
        
        Ort::Value state_ort = Ort::Value::CreateTensor<float>(
                memory_info, _state.data(), _state.size(), state_node_dims, 3);

        // Prepare other inputs and run inference
        Ort::Value sr_ort = Ort::Value::CreateTensor<int64_t>(
                memory_info, &sr, 1, sr_node_dims, 1);

        std::vector<Ort::Value> ort_inputs;

        // Move Ort::Value objects into the vector
        ort_inputs.emplace_back(std::move(input_ort));
        ort_inputs.emplace_back(std::move(state_ort));
        ort_inputs.emplace_back(std::move(sr_ort));

        // Run model
        auto ort_outputs = session->Run(
                Ort::RunOptions{nullptr},
                input_node_names.data(),
                ort_inputs.data(), // Pass the address of the data in the vector
                ort_inputs.size(), // Correct number of inputs
                output_node_names.data(),
                output_node_names.size()
        );

        // 新しいコンテキストとして、連結した入力の末尾 context_size 分を保存する
        _context.assign(input_with_context.end() - context_size, input_with_context.end());

        // Process outputs
        float speech_prob = ort_outputs[0].GetTensorMutableData<float>()[0];
        float *stateN = ort_outputs[1].GetTensorMutableData<float>();
        std::memcpy(_state.data(), stateN, size_state * sizeof(float));
        return speech_prob;

    } catch (const Ort::Exception& e) {
        std::cerr << "Prediction failed: " << e.what() << std::endl;
        return 0.0f; // Handle error appropriately
    }
}
