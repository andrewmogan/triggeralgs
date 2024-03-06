/**
 * @file TriggerActivityMakerTimeWindow.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_TIMEWINDOW_TRIGGERACTIVITYMAKERTIMEWINDOW_HPP_
#define TRIGGERALGS_TIMEWINDOW_TRIGGERACTIVITYMAKERTIMEWINDOW_HPP_

#include "triggeralgs/TriggerActivityFactory.hpp"

namespace triggeralgs {

class TriggerActivityMakerTimeWindow : public TriggerActivityMaker
{
public:
  virtual TriggerActivity& get_current_ta();
  void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas);
  void configure(const nlohmann::json& config);

  bool check_closing_condition(const TriggerPrimitive& tp);
  bool check_skip_condition(const TriggerPrimitive& tp) { return false; }; // Doesn't make use of this condition.
  bool outside_time_window(const TriggerPrimitive& tp);
  void set_ta_attributes(TriggerActivity::Algorithm algo, TriggerActivity::Type type);

  timestamp_t m_window_width;
  timestamp_t m_init_time = 0;

private:
  TriggerActivity m_current_ta;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_TIMEWINDOW_TRIGGERACTIVITYMAKERTIMEWINDOW_HPP_
