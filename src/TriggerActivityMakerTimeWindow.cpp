/**
 * @file TriggerActivityMakerTimeWindow.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/TimeWindow/TriggerActivityMakerTimeWindow.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerTimeWindowPlugin"

namespace triggeralgs {

TriggerActivity&
TriggerActivityMakerTimeWindow::get_current_ta()
{
  return m_current_ta;
}

void
TriggerActivityMakerTimeWindow::set_ta_attributes(TriggerActivity::Algorithm algo, TriggerActivity::Type type)
{
  TriggerActivity& ta = get_current_ta();

  TriggerPrimitive first_tp = ta.inputs.front();
  TriggerPrimitive last_tp = ta.inputs.back();

  ta.channel_start = first_tp.channel;
  ta.channel_end = last_tp.channel;

  ta.time_start = first_tp.time_start;
  ta.time_end = last_tp.time_start;

  ta.detid = first_tp.detid; // Arbitrary choice, but should be the same for all.

  ta.algorithm = algo;
  ta.type = type;

  ta.adc_peak = 0; // Need something to compare to.

  for (const TriggerPrimitive& tp : ta.inputs) {
    ta.adc_integral += tp.adc_integral;
    if (tp.adc_peak <= ta.adc_peak) continue;
    ta.adc_peak = tp.adc_peak;
    ta.channel_peak = tp.channel;
    ta.time_peak = tp.time_peak;
  }
  ta.time_activity = ta.time_peak;
  return;
}

void
TriggerActivityMakerTimeWindow::operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas)
{
  // First TP of run case. Need an initial time.
  if (m_init_time == 0)
    m_init_time = input_tp.time_start;

  // TP closes TA case.
  if (check_closing_condition(input_tp)) {
    // Finalize and push.
    set_ta_attributes(TriggerActivity::Algorithm::kUnknown, TriggerActivity::Type::kTPC);
    output_tas.push_back(m_current_ta);

    // Start a new TA.
    m_current_ta = TriggerActivity();
    m_init_time = input_tp.time_start;
  }

  m_current_ta.inputs.push_back(input_tp);
  return;
}

void TriggerActivityMakerTimeWindow::configure(const nlohmann::json& config)
{
  if (config.is_object() && config.contains("time_window_width"))
    m_window_width = config["time_window_width"];

  TLOG(TLVL_DEBUG_1) << "Using Activity time window width " << m_window_width;
}

bool
TriggerActivityMakerTimeWindow::check_closing_condition(const TriggerPrimitive& tp)
{
  return outside_time_window(tp);
}

bool
TriggerActivityMakerTimeWindow::outside_time_window(const TriggerPrimitive& tp)
{
  return tp.time_start - m_init_time > m_window_width;
}

// Register algo in TA Factory
REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerTimeWindow)

} // namespace triggeralgs
