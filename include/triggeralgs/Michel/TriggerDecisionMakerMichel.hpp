/**
 *  * @file TriggerDecisionMakerMichel.hpp
 *   *
 *    * This is part of the DUNE DAQ Application Framework, copyright 2020.
 *     * Licensing/copyright details are in the COPYING file that you should have
 *      * received with this code.
 *       */

#ifndef TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERDECISIONMAKERMICHEL_HPP_
#define TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERDECISIONMAKERMICHEL_HPP_

#include "triggeralgs/TriggerDecisionMaker.hpp"
#include "triggeralgs/Types.hpp"

#include <algorithm>
#include <atomic>
#include <limits>
#include <vector>

namespace triggeralgs {
class TriggerDecisionMakerMichel : public TriggerDecisionMaker
{


public:

  void operator()(const TriggerCandidate&, std::vector<TriggerDecision>&);

protected:
  std::vector<TriggerCandidate> m_candidate;

  std::atomic<timestamp_t> m_time_window = { 500'000'000 };

  std::atomic<uint16_t> m_threshold = { 1 }; // NOLINT(build/unsigned)

  std::atomic<uint16_t> m_hit_threshold = { 1 }; // NOLINT(build/unsigned)


  void FlushOldCandidate(timestamp_t time_now)
  {
    timestamp_diff_t how_far = time_now - m_time_window;
    auto end = std::remove_if(
                              m_candidate.begin(), m_candidate.end(), [how_far, this](auto& c) -> bool { return (static_cast<timestamp_diff_t>(c.time_start) < how_far); });
    m_candidate.erase(end, m_candidate.end());
  }
};

} // namespace triggeralgs

#endif // TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERDECISIONMAKERMICHEL_HPP_

