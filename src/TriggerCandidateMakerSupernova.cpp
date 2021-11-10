/**
 * @file TriggerCandidateMakerSupernova.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Supernova/TriggerCandidateMakerSupernova.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>

using namespace triggeralgs;
using pd_clock = std::chrono::duration<double, std::ratio<1, 50'000'000>>;

void
TriggerCandidateMakerSupernova::operator()(const TriggerActivity& activity, std::vector<TriggerCandidate>& cand)
{
  timestamp_t time = activity.time_start;
  FlushOldActivity(time); // get rid of old activities in the buffer
  if (activity.tp_list.size() > m_hit_threshold)
    m_activity.push_back(activity);

  // Yay! we have a trigger!
  if (m_activity.size() > m_threshold) {
    std::chrono::time_point<std::chrono::steady_clock> now;

    detid_t detid = WHOLE_DETECTOR;

    std::vector<detid_t> detid_vector; // NOLINT(build/unsigned)
    detid_vector.push_back(detid);
    
    TriggerCandidate trigger{
      time - 500'000'000, // time_start (10 seconds before the start of the activity)
      activity.time_end,  // time_end, but that should probably be _at least_ this number
      time,
      detid_vector,                                // all the detector
      TriggerCandidate::Type::kSupernova, // type ( flag that says what type of trigger might be (e.g. SN/Muon/Beam) )
      TriggerCandidate::Algorithm::kSupernova, // algorithm ( flag that says which algorithm created the trigger (e.g. SN/HE/Solar) )
      0,         // version of the above
      m_activity
    }; // TAs used to form this trigger candidate
    m_activity.clear();
    // Give the trigger word back
    cand.push_back(trigger);
    return;
  }
}
