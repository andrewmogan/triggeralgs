/**
 * @file TriggerCandidateMakerPDSTimeClusteringAlgorithm.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/PDSTimeClustering/TriggerCandidateMakerPDSTimeClusteringAlgorithm.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerCandidateMakerPDSTimeClusteringPlugin"

#include <vector>

using namespace triggeralgs;

using Logging::TLVL_DEBUG_LOW;

void
TriggerCandidateMakerPDSTimeClusteringAlgorithm::operator()(const TriggerActivityPDS& activity, std::vector<TriggerCandidatePDS>& cand)
{ 

  // For now, if there is any single activity from any one detector element, emit
  // a trigger candidate.
  m_activity_count++;
  std::vector<TriggerActivityPDS::TriggerActivityData> ta_list = {static_cast<TriggerActivityPDS::TriggerActivityData>(activity)};

  TLOG_DEBUG(TLVL_DEBUG_LOW) << "[TCM:ADCSW] Emitting an "
                             << dunedaq::trgdataformats::get_trigger_candidate_type_names()[m_tc_type]
                             << " TriggerCandidate with PDSTimeClusteringAlgorithm algorithm" << (m_activity_count-1);

  TriggerCandidatePDS tc;
  tc.time_start = activity.time_start; 
  tc.time_end = activity.time_end;  
  tc.time_candidate = activity.time_activity;
  tc.detid = activity.detid;
  tc.type = m_tc_type;
  tc.algorithm = TriggerCandidatePDS::Algorithm::kPDSTimeClustering;

  tc.inputs = ta_list;

  cand.push_back(tc);

}

void
TriggerCandidateMakerPDSTimeClusteringAlgorithm::configure(const nlohmann::json &config)
{
  // Don't configure if no configuration object
  if (!config.is_object())
    return;

  // Set the TC name
  if (config.contains("tc_type_name")){
    m_tc_type = static_cast<triggeralgs::TriggerCandidatePDS::Type>(
         dunedaq::trgdataformats::string_to_fragment_type_value(config["tc_type_name"]));

    // Set unknown to the default kPDSTimeClustering
    if (m_tc_type == triggeralgs::TriggerCandidatePDS::Type::kUnknown) {
      m_tc_type = TriggerCandidatePDS::Type::kPDSTimeClustering;
    }

    TLOG() << "[TCM:ADCSW]: setting output TC type to "
           << dunedaq::trgdataformats::get_trigger_candidate_type_names()[m_tc_type];
  }
}

REGISTER_TRIGGER_CANDIDATE_MAKER_PDS(TRACE_NAME, TriggerCandidateMakerPDSTimeClusteringAlgorithm)
