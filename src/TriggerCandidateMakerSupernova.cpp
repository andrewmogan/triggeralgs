/**
 * @file TriggerCandidateMakerSupernova.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Supernova/TriggerCandidateMakerSupernova.hpp"

using namespace triggeralgs;

void
TriggerCandidateMakerSupernova::operator()(const TriggerActivity& activity, std::vector<TriggerCandidate>& cand)
{
  timestamp_t time = activity.time_start;
  FlushOldActivity(time); // get rid of old activities in the buffer
  if (activity.inputs.size() > m_hit_threshold)
    m_activity.push_back(activity);

  // Yay! we have a trigger!
  if (m_activity.size() > m_threshold) {

    detid_t detid = dunedaq::trgdataformats::WHOLE_DETECTOR;

    TriggerCandidate tc;
    tc.time_start = time - 500'000'000; // time_start (10 seconds before the start of the activity)
    tc.time_end = activity.time_end;    // time_end; but that should probably be _at least_ this number
    tc.time_candidate = time;
    tc.detid = detid;
    tc.type =
      TriggerCandidate::Type::kSupernova; // type ( flag that says what type of trigger might be (e.g. SN/Muon/Beam) )
    tc.algorithm = TriggerCandidate::Algorithm::kSupernova; // algorithm ( flag that says which algorithm created the
                                                            // trigger (e.g. SN/HE/Solar) )

    for (auto ta : m_activity) {
      tc.inputs.push_back(static_cast<TriggerActivity::TriggerActivityData>(ta));
      tc.regions.insert(ta.region);
    }

    m_activity.clear();
    // Give the trigger word back
    cand.push_back(tc);
  }
}
