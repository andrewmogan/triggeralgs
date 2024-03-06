/**
 * @file TriggerActivityMakerAdjacency.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_ADJACENCY_TRIGGERACTIVITYMAKERADJACENCY_HPP_
#define TRIGGERALGS_ADJACENCY_TRIGGERACTIVITYMAKERADJACENCY_HPP_

#include "triggeralgs/TriggerActivityFactory.hpp"
#include "triggeralgs/TimeWindow/TriggerActivityMakerTimeWindow.hpp"

#include <memory>
#include <cmath>

namespace triggeralgs {
class TriggerActivityMakerAdjacency : public TriggerActivityMakerTimeWindow
{
  public:
    virtual TriggerActivity& get_current_ta();
    void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas);
    void configure(const nlohmann::json& config);
    bool check_closing_condition(const TriggerPrimitive& tp) { return TriggerActivityMakerTimeWindow::check_closing_condition(tp); };
    bool check_skip_condition(const TriggerPrimitive& tp);
    bool tp_is_far(const TriggerPrimitive& tp);
    uint32_t channel_distance(const TriggerPrimitive& tp0, const TriggerPrimitive& tp1);
    void set_ta_attributes();

  private:
    uint32_t m_distance_threshold = 1;
    TriggerActivity m_current_ta;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_ADJACENCY_TRIGGERACTIVITYMAKERADJACENCY_HPP_
