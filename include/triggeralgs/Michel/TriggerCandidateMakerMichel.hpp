/**
 *  * @file TriggerCandidateMakerMichel.hpp
 *   *
 *    * This is part of the DUNE DAQ Application Framework, copyright 2020.
 *     * Licensing/copyright details are in the COPYING file that you should have
 *      * received with this code.
 *       */

#ifndef TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERCANDIDATEMAKERMICHEL_HPP_
#define TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERCANDIDATEMAKERMICHEL_HPP_

#include "triggeralgs/TriggerCandidateMaker.hpp"

#include <algorithm>
#include <atomic>
#include <limits>
#include <vector>

namespace triggeralgs {
class TriggerCandidateMakerMichel : public TriggerCandidateMaker
{


public:

  void operator()(const TriggerActivity&, std::vector<TriggerCandidate>&);

protected:
  std::vector<TriggerActivity> m_activity;

  std::atomic<int64_t> m_time_window = { 500'000'000 };

  std::atomic<uint16_t> m_threshold = { 3 }; // NOLINT(build/unsigned)

  std::atomic<uint16_t> m_hit_threshold = { 2 }; // NOLINT(build/unsigned)


  void FlushOldActivity(timestamp_t time_now)
  {
    timestamp_diff_t how_far = time_now - m_time_window;
    auto end = std::remove_if(
                              m_activity.begin(), m_activity.end(), [how_far, this](auto& c) -> bool { return (static_cast<timestamp_diff_t>(c.time_start) < how_far); });
    m_activity.erase(end, m_activity.end());
  }
};

} // namespace triggeralgs

#endif // TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERCANDIDATEMAKERMICHEL_HPP_

