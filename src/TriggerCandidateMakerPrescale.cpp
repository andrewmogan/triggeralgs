/**
 * @file TriggerCandidateMakerPrescale.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Prescale/TriggerCandidateMakerPrescale.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerCandidateMakerPrescale"

#include <vector>

using namespace triggeralgs;

void
TriggerCandidateMakerPrescale::operator()(const TriggerActivity& activity, std::vector<TriggerCandidate>& cand)
{ 
  if ((m_activity_count++) % m_prescale == 0)
  {
    TLOG(TLVL_DEBUG_1) << "Emitting prescaled TriggerCandidate " << (m_activity_count-1);
    std::vector<detid_t> detid_vector;
    detid_vector.push_back(activity.detid);
    std::vector<TriggerActivity> ta_list;
    ta_list.push_back(activity);
    
    TriggerCandidate tc {
      activity.time_start, 
      activity.time_end,  
      activity.time_start,
      detid_vector,
      TriggerCandidate::Type::kPrescale,
      TriggerCandidate::Algorithm::kPrescale,
      0,
      ta_list
    };
    cand.push_back(tc);
  }
}

void
TriggerCandidateMakerPrescale::configure(const nlohmann::json &config)
{
  //FIXME use some schema here
  if (config.is_object() && config.contains("prescale"))
  {
    m_prescale = config["prescale"]; 
  }
  TLOG_DEBUG(TRACE_NAME) << "Using candidate prescale " << m_prescale;
}
