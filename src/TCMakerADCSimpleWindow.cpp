/**
 * @file TCMakerADCSimpleWindow.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/ADCSimpleWindow/TCMakerADCSimpleWindow.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TCMakerADCSimpleWindowPlugin"

#include <vector>

using namespace triggeralgs;

void
TCMakerADCSimpleWindow::process(const TriggerActivity& activity, std::vector<TriggerCandidate>& cand)
{ 
  // For now, if there is any single activity from any one detector element, emit
  // a trigger candidate.
  std::vector<TriggerActivity::TriggerActivityData> ta_list = {static_cast<TriggerActivity::TriggerActivityData>(activity)};

  TriggerCandidate tc;
  tc.time_start = activity.time_start; 
  tc.time_end = activity.time_end;  
  tc.time_candidate = activity.time_activity;
  tc.detid = activity.detid;
  tc.type = TriggerCandidate::Type::kADCSimpleWindow;
  tc.algorithm = TriggerCandidate::Algorithm::kADCSimpleWindow;

  tc.inputs = ta_list;

  cand.push_back(tc);

}

void
TCMakerADCSimpleWindow::configure(const nlohmann::json &config)
{
  TriggerCandidateMaker::configure(config);
}

REGISTER_TRIGGER_CANDIDATE_MAKER(TRACE_NAME, TCMakerADCSimpleWindow)
