/**
 * @file TriggerActivityMakerPDSTimeClusteringAlgorithm.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/PDSTimeClustering/TriggerActivityMakerPDSTimeClusteringAlgorithm.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerPDSTimeClusteringAlgorithm"

#include <vector>

using namespace triggeralgs;

using Logging::TLVL_DEBUG_MEDIUM;
using Logging::TLVL_IMPORTANT;

void
TriggerActivityMakerPDSTimeClusteringAlgorithm::process(const TriggerPrimitivePDS& input_tp, std::vector<TriggerActivityPDS>& output_ta)
{
  // The first time operator is called, reset
  // window object.
  if(m_current_tpbuffer.is_empty()){
    m_current_tpbuffer.reset(input_tp);
    m_primitive_count++;
    return;
  } 
  
  // If the difference TP is closer than m_ticks_limit ticks from the
  // window, the TP is added to the TP buffer.
  if(input_tp.time_start < (m_current_tpbuffer.time_end + m_ticks_limit)){
//    std::cout << "[TAM:ADCSW] Window not yet complete, adding the input_tp to the window." << std::endl;
    TLOG_DEBUG(TLVL_DEBUG_HIGH) << "[TAM:ADCSW] Window not yet complete, adding the input_tp to the TP buffer.";
    m_current_tpbuffer.add(input_tp);
  }
  // If the TP is further away, we can close the TP buffer (either we save it as a TA or discard it)
  else if(m_current_tpbuffer.adc_integral > m_adc_threshold){
    TLOG_DEBUG(TLVL_DEBUG_LOW) << "[TAM:ADCSW] TP buffer reach the TA condition. TA created.";counter++;
//    std::cout << "[TAM:ADCSW] TP buffer reach the TA condition. TA created.."  << output_ta.size() << " "  << counter << std::endl;
    output_ta.push_back(construct_ta());
    TLOG_DEBUG(TLVL_DEBUG_HIGH) << "[TAM:ADCSW] Resetting buffer with input_tp.";
    m_current_tpbuffer.reset(input_tp);
  }
  else{
    TLOG_DEBUG(TLVL_DEBUG_ALL) << "[TAM:ADCSW] TP buffer did not reach the TA requirement. TP buffer discarded.";
//    std::cout << "[TAM:ADCSW] Window is at required length but adc threshold not met, shifting window along." << std::endl;

    m_current_tpbuffer.reset(input_tp);
  }
  
  TLOG_DEBUG(TLVL_DEBUG_ALL) << "[TAM:ADCSW] " << m_current_tpbuffer;

  m_primitive_count++;

  return;
}

void
TriggerActivityMakerPDSTimeClusteringAlgorithm::configure(const nlohmann::json& config)
{
  TriggerActivityMakerPDS::configure(config);

  //FIXME use some schema here
  if (config.is_object()){
    if (config.contains("tick_limit")) m_ticks_limit = config["tick_limit"];
    if (config.contains("adc_threshold")) m_adc_threshold = config["adc_threshold"];
  }
  else{
    TLOG_DEBUG(TLVL_IMPORTANT) << "[TAM:ADCSW] The DEFAULT values of tick_limit and adc_threshold are being used.";
  }
  TLOG_DEBUG(TLVL_IMPORTANT) << "[TAM:ADCSW] If the total ADC of group of trigger primitives with overlaping times within a tolerance of "
                         << m_ticks_limit << " ticks " << m_adc_threshold << " counts, a TA will be issued.";
}

TriggerActivityPDS
TriggerActivityMakerPDSTimeClusteringAlgorithm::construct_ta() const
{
  TLOG_DEBUG(TLVL_DEBUG_LOW) << "[TAM:ADCSW] I am constructing a trigger activity!";
  //TLOG_DEBUG(TRACE_NAME) << m_current_tpbuffer;

  TriggerPrimitivePDS latest_tp_in_window = m_current_tpbuffer.tp_list.back();
  // The time_peak, time_activity, channel_* and adc_peak fields of this TA are irrelevent
  // for the purpose of this trigger alg.
  TriggerActivityPDS ta;
  ta.time_start = m_current_tpbuffer.time_start;
  ta.time_end = latest_tp_in_window.time_start + latest_tp_in_window.time_over_threshold;
  ta.time_peak = m_current_tpbuffer.time_peak;
  ta.time_activity = latest_tp_in_window.time_peak;
  ta.channel_start = latest_tp_in_window.channel;
  ta.channel_end = latest_tp_in_window.channel;
  ta.channel_peak = latest_tp_in_window.channel;
  ta.adc_integral = m_current_tpbuffer.adc_integral;
  ta.adc_peak = m_current_tpbuffer.adc_peak;
  ta.detid = latest_tp_in_window.detid;
  ta.type = TriggerActivityPDS::Type::kPDS;
  ta.algorithm = TriggerActivityPDS::Algorithm::kADCSimpleWindow;
  ta.inputs = m_current_tpbuffer.tp_list;
  return ta;
}

// Register algo in TA Factory
REGISTER_TRIGGER_ACTIVITY_MAKER_PDS(TRACE_NAME, TriggerActivityMakerPDSTimeClusteringAlgorithm)
