/**
 * @file TCMakerPDSTimeClusteringAlgorithm.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/PDSTimeClustering/TCMakerPDSTimeClusteringAlgorithm.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TCMakerPDSTimeClusteringAlgorithm"

#include <vector>

using namespace triggeralgs;

void
TCMakerPDSTimeClusteringAlgorithm::process(const TriggerActivityPDS& activity, std::vector<TriggerCandidatePDS>& cand)
{ 
  // For now, if there is any single activity from any one detector element, emit
  // a trigger candidate.
  std::vector<TriggerActivity::TriggerActivityData> ta_list = {static_cast<TriggerActivity::TriggerActivityData>(activity)};

  TriggerCandidatePDS tc;
  tc.time_start = activity.time_start; 
  tc.time_end = activity.time_end;  
  tc.time_candidate = activity.time_activity;
  tc.detid = activity.detid;
  tc.type = m_tc_type_out; 
  tc.algorithm = TriggerCandidate::Algorithm::kADCSimpleWindow;

  tc.inputs = ta_list;

  cand.push_back(tc);

}

void
TCMakerPDSTimeClusteringAlgorithm::configure(const nlohmann::json &config)
{
  TriggerCandidateMakerPDS::configure(config);
}

REGISTER_TRIGGER_CANDIDATE_MAKER(TRACE_NAME, TCMakerPDSTimeClusteringAlgorithm)
