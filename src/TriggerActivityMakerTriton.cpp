/**
 * @file TriggerActivityMakerTriton.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */


#include "triggeralgs/Triton/TriggerActivityMakerTriton.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerTritonPlugin"

namespace tc = triton::client;

namespace triggeralgs {

using triton_utils::fail_if_error;
using triton_utils::warn_if_error;
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

  // Reset the current.
  m_current_ta = TriggerActivity();
  return;
}

std::vector<std::vector<std::vector<int>>>
TriggerActivityMakerTriton::get_adcs_from_trigger_primitives(
  const uint64_t number_tps,
  const uint64_t number_time_ticks,
  const uint64_t number_wires) {

  std::vector<std::vector<std::vector<int>>> adc_values(
    number_tps,
    std::vector<std::vector<int>>(
      number_time_ticks, 
      std::vector<int>(number_wires, 0)
    )
  );
  return adc_values;
}

void 
TriggerActivityMakerTriton::configure(const nlohmann::json& config)
{
  if (config.is_object() && config.contains("number_tps_per_request")) {
    m_number_tps_per_request = config["number_tps_per_request"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured TPs per request: " << m_number_tps_per_request;
  }
  if (config.is_object() && config.contains("batch_size")) {
    m_batch_size = config["batch_size"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured batch size: " << m_batch_size;
  }
  if (config.is_object() && config.contains("number_time_ticks")) {
    m_number_time_ticks = config["number_time_ticks"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured time ticks per TP: " << m_number_time_ticks;
  }
  if (config.is_object() && config.contains("number_wires")) {
    m_number_wires = config["number_wires"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured number of wires per TP: " << m_number_wires;
  }
  if (config.is_object() && config.contains("inference_url")) {
    m_inference_url = config["inference_url"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Inference URL is " << m_inference_url;
  }
  if (config.is_object() && config.contains("model_name")) {
    m_model_name = config["model_name"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Model name is " << m_model_name;
  }
  if (config.is_object() && config.contains("model_version")) {
    m_model_version = config["model_version"];
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

  std::unique_ptr<TritonClient> triton_client;

  //fail_if_error(tc::InferenceServerGrpcClient::Create(&client, m_inference_url), "Could not create Triton client");

}

REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerTriton)
} // namespace triggeralgs

