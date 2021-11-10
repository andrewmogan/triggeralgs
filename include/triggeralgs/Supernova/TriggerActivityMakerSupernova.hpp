/**
 * @file TriggerActivityMakerSupernova.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SRC_TRIGGERALGS_SUPERNOVA_TRIGGERACTIVITYMAKERSUPERNOVA_HPP_
#define TRIGGERALGS_SRC_TRIGGERALGS_SUPERNOVA_TRIGGERACTIVITYMAKERSUPERNOVA_HPP_

#include "triggeralgs/TriggerActivityMaker.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace triggeralgs {
class TriggerActivityMakerSupernova : public TriggerActivityMaker
{

  /// This activity maker makes an activity with all the trigger primitives
  inline bool is_time_consistent(const TriggerPrimitive& input_tp) const
  {

    timestamp_t tend = input_tp.time_start + input_tp.time_over_threshold;

    bool is_close_to_edge = (m_time_tolerance > abs(timestamp_diff_t(input_tp.time_start) - timestamp_diff_t(m_time_end))) ||
      m_time_tolerance > abs(timestamp_diff_t(input_tp.time_start) - timestamp_diff_t(m_time_start)) ||
      m_time_tolerance > abs(timestamp_diff_t(tend) - timestamp_diff_t(m_time_end)) ||
      m_time_tolerance > abs(timestamp_diff_t(tend) - timestamp_diff_t(m_time_start));

    bool is_in_between_edge = ((tend > m_time_start && tend < m_time_end) ||
                               (input_tp.time_start > m_time_start && input_tp.time_start < m_time_end));

    return is_in_between_edge || is_close_to_edge;
  }

  inline bool is_channel_consistent(const TriggerPrimitive& input_tp) const
  {

    bool is_close_to_edge = (m_channel_tolerance > abs(input_tp.channel - m_channel_end) ||
                             m_channel_tolerance > abs(input_tp.channel - m_channel_start));

    bool is_in_between_edge = (input_tp.channel > m_channel_start && input_tp.channel < m_channel_end);

    return is_in_between_edge || is_close_to_edge;
  }

public:
  void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta) override;

  void flush(timestamp_t, std::vector<TriggerActivity>& tas) override { tas.push_back(MakeTriggerActivity()); }

protected:
  timestamp_diff_t m_time_tolerance =
    250; /// Maximum tolerated time difference between two primitives to form an activity (in 50 MHz clock ticks)
  channel_t m_channel_tolerance =
    2; /// Maximum tolerated channel number difference between two primitives to form an activity

private:
  TriggerActivity MakeTriggerActivity() const
  {
    TriggerActivity ta{ m_time_start,  m_time_end,     m_time_peak,    m_time_activity, m_channel_start,
                        m_channel_end, m_channel_peak, m_adc_integral, m_adc_peak,      m_detid,
                        m_type,        m_algorithm,    m_version,      m_tp_list };
    return ta;
  }

  timestamp_t m_time_start = 0;
  timestamp_t m_time_end = 0;
  timestamp_t m_time_peak = 0;
  timestamp_t m_time_activity = 0;
  channel_t m_channel_start = 0; // NOLINT(build/unsigned)
  channel_t m_channel_end = 0;   // NOLINT(build/unsigned)
  channel_t m_channel_peak = 0;  // NOLINT(build/unsigned)
  uint64_t m_adc_integral = 0;  // NOLINT(build/unsigned)
  uint16_t m_adc_peak = 0;      // NOLINT(build/unsigned)
  detid_t m_detid = 0;         // NOLINT(build/unsigned)
  TriggerActivity::Type m_type = TriggerActivity::Type::kTPC;
  TriggerActivity::Algorithm m_algorithm = TriggerActivity::Algorithm::kSupernova;
  version_t m_version = 0;

  std::vector<TriggerPrimitive> m_tp_list;
};
} // namespace triggeralgs

#endif // TRIGGERALGS_SRC_TRIGGERALGS_SUPERNOVA_TRIGGERACTIVITYMAKERSUPERNOVA_HPP_
