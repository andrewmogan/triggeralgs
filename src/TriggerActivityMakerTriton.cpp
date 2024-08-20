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

#define FAIL_IF_ERR(X, MSG)                                        \
  {                                                                \
    tc::Error err = (X);                                           \
    if (!err.IsOk()) {                                             \
      std::cerr << "error: " << (MSG) << ": " << err << std::endl; \
      exit(1);                                                     \
    }                                                              \
  }

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
  // Check is the server is live
  bool live;
  tc::Error err = client->IsServerLive(&live);
  if (!err.IsOk()) {
    fail_if_error(err, "Unable to get server liveness");
  }
  if (live) {
    TLOG(TLVL_DEBUG_INFO) << "[TA:Triton] Triton server is live";
  }

  // Server metadata
  inference::ServerMetadataResponse server_metadata;
  err = client->ServerMetadata(&server_metadata);
  if (!err.IsOk()) {
    fail_if_error(err, "Unable to get server metadata");
  }
  if (server_metadata.name().compare("triton") != 0) {
    std::cerr << "error: unexpected server metadata: "
              << server_metadata.DebugString() << std::endl;
    exit(1);
  }

  TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TAM:Triton] Server metadata debug string: " << server_metadata.DebugString();

  return;
}

void TriggerActivityMakerTriton::check_model_readiness(const std::string model_name, const std::string model_version) const {
  bool model_ready;
  tc::Error model_load_err;
  model_load_err = client->IsModelReady(&model_ready, model_name, model_version);
  if (!model_load_err.IsOk()) {
    fail_if_error(model_load_err, "Unable to get model readiness");
  }
  if (model_ready) {
    TLOG(TLVL_DEBUG_INFO) << "Model " << model_name << " is ready.";
  }

  if (!model_ready) {
    std::cerr << "error: model " << model_name << " is not live" << std::endl;
    exit(1);
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

  tc::Error server_create_err = tc::InferenceServerGrpcClient::Create(&client, m_inference_url);
  if (!server_create_err.IsOk()) {
    fail_if_error(server_create_err, "Could not create Triton client");
  }
  else {
    TLOG(TLVL_DEBUG_INFO) << "Triton client is live and communicating with server on " << m_inference_url;
  }

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

