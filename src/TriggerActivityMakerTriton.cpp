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

using Logging::TLVL_IMPORTANT;
using Logging::TLVL_DEBUG_HIGH;

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

  // Reset the current.
  m_current_ta = TriggerActivity();
}

void
TriggerActivityMakerTriton::configure(const nlohmann::json& config)
{
  if (config.is_object() && config.contains("number_tps_per_request")) {
    m_number_tps_per_request = config["number_tps_per_request"];
    TLOG_DEBUG(TLVL_DEBUG_HIGH) << "[TA:Triton] Emitting Triton TA with " << m_current_ta.inputs.size() << " TPs.";
  }
  if (config.is_object() && config.contains("inference_url")) {
    m_inference_url = config["inference_url"];
    TLOG() << "[TA:Triton] Inference URL is " << m_inference_url;
  }
  if (config.is_object() && config.contains("print_tp_info")) {
    m_print_tp_info = config["print_tp_info"];
    TLOG_DEBUG(TLVL_DEBUG_HIGH) << "[TA:Triton] Printing of TP info enabled";
  }
}

REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerTriton)
} // namespace triggeralgs

