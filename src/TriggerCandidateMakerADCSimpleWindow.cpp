/**
 * @file TriggerCandidateMakerADCSimpleWindow.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/ADCSimpleWindow/TriggerCandidateMakerADCSimpleWindow.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerCandidateMakerADCSimpleWindow"

#include <vector>

using namespace triggeralgs;

void
TriggerCandidateMakerADCSimpleWindow::operator()(const TriggerActivity& activity, std::vector<TriggerCandidate>& cand)
{ 

  // For now, if there is any single activity from any one detector element, emit
  // a trigger candidate.
  m_activity_count++;
  std::vector<TriggerActivity::TriggerActivityData> ta_list = {static_cast<TriggerActivity::TriggerActivityData>(activity)};

  TLOG(TLVL_DEBUG_1) << "Emitting an ADCSimpleWindow TriggerCandidate " << (m_activity_count-1);
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
TriggerCandidateMakerADCSimpleWindow::configure(const nlohmann::json &config)
{
}

