/**
 * @file TriggerActivityMakerTriton.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */


#include "triggeralgs/Triton/TriggerActivityMakerTriton.hpp"
#include "triggeralgs/Triton/ModelInputPreparer.hpp"

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

  const std::string model_name = triton_client->get_model_name();

  const std::unordered_map<std::string, ModelIOHandler> handlers = triggeralgs::get_model_io_handlers();
  //std::map<std::string, ModelIOHandler>::iterator handler = handlers.find(model_name);
  auto it = handlers.find(model_name);
  if (it == handlers.end()) {
    std::cerr << "No model IO handler registered for model: " << model_name << std::endl;
    exit(1);
  }

  ModelIOHandler handler = it->second;
  handler.prepare_input(*triton_client);

  triton_client->dispatch();

  handler.handle_output(*triton_client);

  triton_client->reset();
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
  if (config.is_object()) {
    if (config.contains("number_tps_per_request")) {
      m_number_tps_per_request = config["number_tps_per_request"];
      TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured TPs per request: " << m_number_tps_per_request;
    }
    if (config.contains("batch_size")) {
      m_batch_size = config["batch_size"];
      TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured batch size: " << m_batch_size;
    }
    if (config.contains("number_time_ticks")) {
      m_number_time_ticks = config["number_time_ticks"];
      TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured time ticks per TP: " << m_number_time_ticks;
    }
    if (config.contains("number_wires")) {
      m_number_wires = config["number_wires"];
      TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured number of wires per TP: " << m_number_wires;
    }
    if (config.contains("print_tp_info")) {
      m_print_tp_info = config["print_tp_info"];
      TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Printing of TP info enabled";
    }
    if (config.contains("verbose")) {
      m_verbose = config["verbose"];
      TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Verbose output enabled";
    }
    if (config.contains("outputs")) {
      //m_outputs = config["outputs"];
      m_outputs.emplace_back(config["outputs"]);
      TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Outputs";
    }
  }

  TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Using configuration:\n" << config.dump(4);

  //std::unique_ptr<TritonClient> triton_client;
  triton_client = std::make_unique<TritonClient>(config);
}

REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerTriton)
} // namespace triggeralgs

