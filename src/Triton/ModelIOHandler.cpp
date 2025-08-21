#include "triggeralgs/Triton/ModelIOHandler.hpp"
#include "triggeralgs/TriggerPrimitive.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>


namespace triggeralgs {

const std::unordered_map<std::string, ModelIOHandler>& get_model_io_handlers() {
  static std::unordered_map<std::string, ModelIOHandler> handlers = {
    { "simple",  { simple_input_preparer, simple_output_handler } },
    { "adc_cm",  { adc_cm_input_preparer, adc_cm_output_handler } },
    { "adc_tm",  { adc_tm_input_preparer, adc_tm_output_handler } },
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

// Channel-major: shape = {1280, 2000}
void adc_cm_input_preparer(TritonClient& client)
{
  client.set_batch_size(1);

  std::vector<std::vector<int32_t>> batch(1);
  batch[0].assign(1280 * 2000, 0);  // synthetic payload

  auto& in0 = client.input().at("INPUT0");
  in0.set_shape({1280, 2000});
  in0.to_server<int32_t>(std::make_shared<decltype(batch)>(std::move(batch)));
}

void adc_cm_output_handler(TritonClient&) {}

// Time-major: shape = {2000, 1280}
void adc_tm_input_preparer(TritonClient& client)
{
  client.set_batch_size(1);

  std::vector<std::vector<int32_t>> batch(1);
  batch[0].assign(1280 * 2000, 0);  // synthetic payload

  auto& in0 = client.input().at("INPUT0");
  in0.set_shape({2000, 1280});
  in0.to_server<int32_t>(std::make_shared<decltype(batch)>(std::move(batch)));
}

void adc_tm_output_handler(TritonClient&){}


} // namespace triggeralgs
