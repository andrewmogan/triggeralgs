#include "triggeralgs/Triton/ModelIOHandler.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>

namespace triggeralgs {

const std::unordered_map<std::string, ModelIOHandler>& get_model_io_handlers() {
  static std::unordered_map<std::string, ModelIOHandler> handlers = {
    {
      "simple",
      {
        simple_input_preparer,
        simple_output_handler
      }
    },
    {
      "another_model",
      {
        [](TritonClient& client) {
          // Prepare inputs for fancy_model
        },
        [](TritonClient& client) {
          // Handle outputs for fancy_model
        }
      }
    }
  };

  return handlers;
}

void simple_input_preparer(TritonClient& triton_client) {
    // This toy model checks the sum and difference of two vectors length 16
    const size_t input_size_per_batch = 16;
    std::vector<int32_t> input0_data(input_size_per_batch);
    std::vector<int32_t> input1_data(input_size_per_batch);
    for (size_t i = 0; i < input_size_per_batch; ++i) {
        input0_data[i] = i;
        input1_data[i] = 1;
    }

    const unsigned batch_size = triton_client.get_batch_size();
    TLOG() << "Confirm batch size: " << batch_size << " ";
    //triton_client.set_batch_size(1);

    TritonData<tc::InferInput>& input0 = triton_client.input().at("INPUT0");
    TritonData<tc::InferInput>& input1 = triton_client.input().at("INPUT1");

    auto data0 = std::make_shared<triggeralgs::TritonInput<int32_t>>();
    auto data1 = std::make_shared<triggeralgs::TritonInput<int32_t>>();
    data0->reserve(batch_size);
    data1->reserve(batch_size);

    for (size_t batch_idx = 0; batch_idx < batch_size; ++batch_idx) {
      std::vector<int32_t> input0_data(input_size_per_batch);
      std::vector<int32_t> input1_data(input_size_per_batch);
      
      for (size_t i = 0; i < input_size_per_batch; ++i) {
          input0_data[i] = i + batch_idx * 100;  // Distinguishable data per batch
          input1_data[i] = 1;
      }

      data0->emplace_back(input0_data);
      data1->emplace_back(input1_data);
    }

    //data0->emplace_back(input0_data);
    //data1->emplace_back(input1_data);
    input0.to_server(data0);
    input1.to_server(data1);

    std::cout << "Prepared inputs for SimpleModel" << std::endl;
}

void simple_output_handler(TritonClient& client)
{
  const auto& out0 = client.output().at("OUTPUT0").from_server<int32_t>();
  const auto& out1 = client.output().at("OUTPUT1").from_server<int32_t>();
  for (size_t i = 0; i < out0[0].size(); ++i) {
    TLOG() << "Sum: " << out0[0][i];
    TLOG() << "Diff: " << out1[0][i];
  }
}

// Complex model input preparation logic
void complex_model_preparer(TritonClient& triton_client, const std::string& model_name) {
    std::vector<int64_t> input_data(100); 
    for (size_t i = 0; i < 100; ++i) {
        input_data[i] = i * 2;  // Example data manipulation
    }

    triton_client.set_batch_size(1);

    TritonData<tc::InferInput>& input0 = triton_client.input().at("INPUT0");

    auto data0 = std::make_shared<triggeralgs::TritonInput<int64_t>>();
    data0->reserve(1);

    data0->emplace_back(input_data);
    input0.to_server(data0);

    std::cout << "Prepared inputs for ComplexModel" << std::endl;
}

} // namespace triggeralgs
