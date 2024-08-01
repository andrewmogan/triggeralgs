/**
 * @file TriggerActivityMakerTriton.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Triton/TriggerActivityMakerTriton.hpp"

#include "TRACE/trace.h"
#include "grpc_client.h"
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

  query_triton_server(m_current_ta, m_inference_url);

  // Reset the current.
  m_current_ta = TriggerActivity();
  return;
}

void query_triton_server(const TriggerActivity& trigger_activity, const std::string inference_url) {
  TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TAM:Triton] Querying server with TA size " << trigger_activity.inputs.size();
  
  // Create the server
  std::unique_ptr<triton::client::InferenceServerGrpcClient> client;
  FAIL_IF_ERR(tc::InferenceServerGrpcClient::Create(&client, inference_url), err);

  // Check is the server is live
  bool live;
  FAIL_IF_ERR(
      client->IsServerLive(&live),
      "unable to get server liveness");
  if (!live) {
    std::cerr << "error: server is not live" << std::endl;
    exit(1);
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
  if (config.is_object() && config.contains("print_tp_info")) {
    m_print_tp_info = config["print_tp_info"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Printing of TP info enabled";
  }
}

REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerTriton)
} // namespace triggeralgs

