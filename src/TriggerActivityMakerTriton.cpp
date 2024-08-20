/**
 * @file TriggerActivityMakerTriton.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Triton/TriggerActivityMakerTriton.hpp"

#include "TRACE/trace.h"
//#include "grpc_client.h"
//#include "triggeralgs/Triton/json_utils.h"
//#include "rapidjson/document.h"
#define TRACE_NAME "TriggerActivityMakerTritonPlugin"

namespace tc = triton::client;

namespace triggeralgs {

using Logging::TLVL_GENERAL;
using Logging::TLVL_DEBUG_INFO;
using Logging::TLVL_DEBUG_HIGH;
using Logging::TLVL_DEBUG_ALL;

static std::unique_ptr<triton::client::InferenceServerGrpcClient> client;

void
TriggerActivityMakerTriton::operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas)
{
  // Expect that TPs are inherently time ordered.
  m_current_ta.inputs.push_back(input_tp);

  // Add useful info about recived TPs here for FW and SW TPG guys.
  if (m_print_tp_info) {
    TLOG_DEBUG(TLVL_DEBUG_ALL) << "[TAM:Triton] TP Start Time: " << input_tp.time_start << ", TP ADC Sum: " <<  input_tp.adc_integral
	    << ", TP TOT: " << input_tp.time_over_threshold << ", TP ADC Peak: " << input_tp.adc_peak
     	<< ", TP Offline Channel ID: " << input_tp.channel;
  }

  if (m_current_ta.inputs.size() < m_number_tps_per_request) {
    m_current_ta.inputs.push_back(input_tp);
    return;
  }

  //check_triton_server_liveness(m_inference_url);
  //check_model_inputs(m_model_name, m_model_version);


  // Reset the current.
  m_current_ta = TriggerActivity();
  return;
}

void TriggerActivityMakerTriton::check_triton_server_liveness(const std::string& inference_url) const {
  bool live;
  fail_if_error(client->IsServerLive(&live), "Unable to get server liveness");

  // Server metadata
  inference::ServerMetadataResponse server_metadata;
  fail_if_error(client->ServerMetadata(&server_metadata), "Unable to get server metadata");
  if (server_metadata.name().compare("triton") != 0) {
    std::cerr << "Error: unexpected server metadata: "
              << server_metadata.DebugString() << std::endl;
    exit(1);
  }

  TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TAM:Triton] Server metadata debug string: " << server_metadata.DebugString();

  return;
}

void TriggerActivityMakerTriton::check_model_readiness(const std::string model_name, const std::string model_version) const {
  bool model_ready;
  fail_if_error(client->IsModelReady(&model_ready, model_name, model_version), "Unable to get model readiness");
  if (model_ready) {
    TLOG(TLVL_DEBUG_INFO) << "Model " << model_name << " is ready.";
  }

  return;
}

void TriggerActivityMakerTriton::check_model_inputs(const std::string model_name, const std::string model_version) const {

  /*
  if (model_name != "simple") {
    TLOG_DEBUG(TLVL_IMPORTANT) << "WARNING: Model name is not 'simple'; skipping inference reuqest";
    return;
  }
  */
  
  // Placeholder using the "simple" model
  std::vector<int32_t> input0_data(16);
  std::vector<int32_t> input1_data(16);
  for (size_t i = 0; i < 16; ++i) {
    input0_data[i] = i;
    input1_data[i] = 1;
  }

  std::vector<int64_t> shape{1, 16};

  // Initialize the inputs with the data.
  tc::InferInput* input0;
  tc::InferInput* input1;

  bool model_ready;
  tc::Error model_load_err;
  model_load_err = client->IsModelReady(&model_ready, model_name, model_version);
  fail_if_error(client->IsModelReady(&model_ready, model_name, model_version), "Unable to get model readiness");

  fail_if_error(tc::InferInput::Create(&input0, "INPUT0", shape, "INT32"), "Unable to get INPUT0");
  std::shared_ptr<tc::InferInput> input0_ptr;
  input0_ptr.reset(input0);

  fail_if_error(tc::InferInput::Create(&input1, "INPUT1", shape, "INT32"), "Unable to get INPUT1");
  std::shared_ptr<tc::InferInput> input1_ptr;
  input1_ptr.reset(input1);

  fail_if_error(
    input0_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&input0_data[0]),
          input0_data.size() * sizeof(int32_t)),
    "Unable to set data for INPUT0");
  fail_if_error(
    input1_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&input1_data[0]),
          input1_data.size() * sizeof(int32_t)),
    "Unable to set data for INPUT1");

  // Generate the outputs to be requested.
  tc::InferRequestedOutput* output0;
  tc::InferRequestedOutput* output1;

  fail_if_error(
    tc::InferRequestedOutput::Create(&output0, "OUTPUT0"), 
    "Unable to get OUTPUT0");
  std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
  output0_ptr.reset(output0);

  fail_if_error(
    tc::InferRequestedOutput::Create(&output1, "OUTPUT1"), 
    "Unable to get OUTPUT1");
  std::shared_ptr<tc::InferRequestedOutput> output1_ptr;
  output1_ptr.reset(output1);

  // The inference settings. Will be using default for now.
  tc::InferOptions options(model_name);
  options.model_version_ = model_version;
  options.client_timeout_ = m_client_timeout_microseconds;

  std::vector<tc::InferInput*> inputs = {input0_ptr.get(), input1_ptr.get()};
  std::vector<const tc::InferRequestedOutput*> outputs = {
      output0_ptr.get(), output1_ptr.get()
  };

  tc::InferResult* results;
  fail_if_error(
    client->Infer(&results, options, inputs, outputs), 
    "Unable to run model");
  std::shared_ptr<tc::InferResult> results_ptr;
  results_ptr.reset(results);

  // Get pointers to the result returned...
  int32_t* output0_data;
  size_t output0_byte_size;
  fail_if_error(
    results_ptr->RawData("OUTPUT0", (const uint8_t**)&output_data, &output0_byte_size),
    "Unable to get result data for OUTPUT0");
  if (output0_byte_size != 64) {
    std::cerr << "error: received incorrect byte size for 'OUTPUT0': "
              << output0_byte_size << std::endl;
    exit(1);
  }

  int32_t* output1_data;
  size_t output1_byte_size;
  fail_if_error(
    results_ptr->RawData("OUTPUT1", (const uint8_t**)&output_data, &output0_byte_size),
    "Unable to get result data for output1");
  if (output1_byte_size != 64) {
    std::cerr << "error: received incorrect byte size for 'OUTPUT1': "
              << output1_byte_size << std::endl;
    exit(1);
  }

  for (size_t i = 0; i < 16; ++i) {
    std::cout << input0_data[i] << " + " << input1_data[i] << " = "
              << *(output0_data + i) << std::endl;
    std::cout << input0_data[i] << " - " << input1_data[i] << " = "
              << *(output1_data + i) << std::endl;

    if ((input0_data[i] + input1_data[i]) != *(output0_data + i)) {
      std::cerr << "Error: incorrect sum" << std::endl;
      exit(1);
    }
    if ((input0_data[i] - input1_data[i]) != *(output1_data + i)) {
      std::cerr << "Error: incorrect difference" << std::endl;
      exit(1);
    }
  }

  return;
}

void
TriggerActivityMakerTriton::configure(const nlohmann::json& config)
{
  if (config.is_object() && config.contains("number_tps_per_request")) {
    m_number_tps_per_request = config["number_tps_per_request"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Emitting Triton TA with " << m_current_ta.inputs.size() << " TPs.";
  }
  if (config.is_object() && config.contains("inference_url")) {
    m_inference_url = config["inference_url"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Inference URL is " << m_inference_url;
  }
  if (config.is_object() && config.contains("model_name")) {
    m_inference_url = config["model_name"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Model name is " << m_model_name;
  }
  if (config.is_object() && config.contains("model_version")) {
    m_inference_url = config["model_version"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Model version is " << m_model_version;
  }
  if (config.is_object() && config.contains("client_timeout_microseconds")) {
    m_client_timeout_microseconds = config["client_timeout_microseconds"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] client_timeout_microseconds is " << m_client_timeout_microseconds;
  }
  if (config.is_object() && config.contains("server_timeout_microseconds")) {
    m_server_timeout_microseconds = config["server_timeout_microseconds"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] server_timeout_microseconds is " << m_server_timeout_microseconds;
  }
  if (config.is_object() && config.contains("print_tp_info")) {
    m_print_tp_info = config["print_tp_info"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Printing of TP info enabled";
  }

  TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Using configuration:\n" << config.dump(4);

  fail_if_error(tc::InferenceServerGrpcClient::Create(&client, m_inference_url), "Could not create Triton client");
  TLOG(TLVL_DEBUG_INFO) << "Triton client is live and communicating with server on " << m_inference_url;

  check_triton_server_liveness(m_inference_url);
  check_model_readiness(m_model_name, m_model_version);
  check_model_inputs(m_model_name, m_model_version);
}

void TriggerActivityMakerTriton::fail_if_error(const tc::Error& err, const std::string& msg) const {
  if (!err.IsOk()) {
    TLOG() << "[TA:Triton] ERROR: " << msg << ": " << err << std::endl;
  exit(1);
  }
}

REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerTriton)
} // namespace triggeralgs

