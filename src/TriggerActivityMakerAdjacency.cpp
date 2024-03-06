/**
 * @file TriggerActivityMakerAdjacency.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you shuold have
 * received with this code
 */

#include "triggeralgs/Adjacency/TriggerActivityMakerAdjacency.hpp"
#include <iostream>

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerAdjacencyPlugin"

namespace triggeralgs {

TriggerActivity&
TriggerActivityMakerAdjacency::get_current_ta()
{
  return m_current_ta;
}

bool TriggerActivityMakerAdjacency::check_skip_condition(const TriggerPrimitive& tp)
{
  return tp_is_far(tp) || TriggerActivityMakerTimeWindow::check_skip_condition(tp);
}

uint32_t TriggerActivityMakerAdjacency::channel_distance(const TriggerPrimitive& tp0, const TriggerPrimitive& tp1)
{
  return std::abs(tp0.channel - tp1.channel);
}

bool TriggerActivityMakerAdjacency::tp_is_far(const TriggerPrimitive& new_tp)
{
  TriggerActivity ta = get_current_ta();
  // Check the distance to current TPs
  for (const TriggerPrimitive& tp : ta.inputs) {
    // If it is in range from any TP, return true.
    if (channel_distance(tp, new_tp) < m_distance_threshold)
      return false;
  }
  return true;
}

void TriggerActivityMakerAdjacency::set_ta_attributes()
{
  TriggerActivityMakerTimeWindow::set_ta_attributes(TriggerActivity::Algorithm::kUnknown, TriggerActivity::Type::kTPC);
  return;
}

void TriggerActivityMakerAdjacency::operator()(
    const TriggerPrimitive& input_tp,
    std::vector<TriggerActivity>& output_tas)
{
  if (m_init_time == 0) {
    m_init_time = input_tp.time_start;
    m_current_ta.inputs.push_back(input_tp);
    return;
  }

  if (check_closing_condition(input_tp)) {
    if (m_current_ta.inputs.size() > 20) { // TODO: Make this configurable.
      set_ta_attributes();
      output_tas.push_back(m_current_ta);
    }
    m_current_ta = TriggerActivity();
    m_current_ta.inputs.push_back(input_tp);
    m_init_time = input_tp.time_start;
    return;
  }

  if (check_skip_condition(input_tp)) {
    TLOG(TLVL_DEBUG_1) << "Skipping a TP.";
    return;
  }

  m_current_ta.inputs.push_back(input_tp);
}

void TriggerActivityMakerAdjacency::configure(const nlohmann::json& config)
{
  TriggerActivityMakerTimeWindow::configure(config);
  if (config.is_object() && config.contains("distance_threshold"))
    m_distance_threshold = config["distance_threshold"];

  TLOG(TLVL_DEBUG_1) << "Using Activity distance threshold " << m_distance_threshold;
  return;
}

// Register algo in TA Factory
REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerAdjacency)

} // namespace triggeralgs
