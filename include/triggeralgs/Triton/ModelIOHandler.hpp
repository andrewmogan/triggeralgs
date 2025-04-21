#ifndef TRIGGERALGS_INCLUDE_TRITON_MODELINPUTPREPARER_HPP
#define TRIGGERALGS_INCLUDE_TRITON_MODELINPUTPREPARER_HPP

#include "triggeralgs/Triton/TritonClient.hpp"

#include <unordered_map>
#include <functional>
#include <string>

namespace triggeralgs {

struct ModelIOHandler {
  std::function<void(TritonClient&)> prepare_input;
  std::function<void(TritonClient&)> handle_output;
};

// Central registry accessor
const std::unordered_map<std::string, ModelIOHandler>& get_model_io_handlers();

// Declare input preparers and output handlers for your model here
void simple_input_preparer(TritonClient& client);
void simple_output_handler(TritonClient& client);

} // namespace triggeralgs

#endif