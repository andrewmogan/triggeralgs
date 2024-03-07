/**
 * @file TriggerActivityMakerChannelDistance.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/ChannelDistance/TriggerActivityMakerChannelDistance.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerChannelDistancePlugin"

namespace triggeralgs {

void
TriggerActivityMakerChannelDistance::operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas)
{
  // Start a new TA if not already going.
  if (m_current_ta.inputs.empty()) {
    m_current_ta.inputs.push_back(input_tp);
    return;
  }

  // Check to close the TA.
  if (m_closer->close_logic(input_tp, m_current_ta)) {
    // Check to block the TA.
    if (!m_blocker->block_logic(m_current_ta)) {
      set_ta_attributes();
      output_tas.push_back(m_current_ta);
    }
    m_current_ta = TriggerActivity();
    m_current_ta.inputs.push_back(input_tp);
    return;
  }

  // Check to skip the TP.
  if (m_skipper->skip_logic(input_tp, m_current_ta)) {
    return;
  }

  m_current_ta.inputs.push_back(input_tp);
}

void
TriggerActivityMakerChannelDistance::configure(const nlohmann::json& config)
{
  m_blocker->configure(config);
  m_closer->configure(config);
  m_skipper->configure(config);
}

void
TriggerActivityMakerChannelDistance::set_ta_attributes()
{
  TriggerPrimitive first_tp = m_current_ta.inputs.front();
  TriggerPrimitive last_tp = m_current_ta.inputs.back();

  m_current_ta.channel_start = first_tp.channel;
  m_current_ta.channel_end = last_tp.channel;

  m_current_ta.time_start = first_tp.time_start;
  m_current_ta.time_end = last_tp.time_start;

  m_current_ta.detid = first_tp.detid;

  m_current_ta.algorithm = TriggerActivity::Algorithm::kUnknown;
  m_current_ta.type = TriggerActivity::Type::kTPC;

  m_current_ta.adc_peak = 0;
  for (const TriggerPrimitive& tp : m_current_ta.inputs) {
    m_current_ta.adc_integral += tp.adc_integral;
    if (tp.adc_peak <= m_current_ta.adc_peak) continue;
    m_current_ta.adc_peak = tp.adc_peak;
    m_current_ta.channel_peak = tp.channel;
    m_current_ta.time_peak = tp.time_peak;
  }
  m_current_ta.time_activity = m_current_ta.time_peak;
}

// Register algo in TA Factory
REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerChannelDistance)

} // namespace triggeralgs
