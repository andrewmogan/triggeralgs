/**
 * @file TriggerCandidateMakerMichel.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Michel/TriggerCandidateMakerMichel.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>




using namespace triggeralgs;
using namespace std;
void
TriggerCandidateMakerMichel::operator()(const TriggerActivity& activity, std::vector<TriggerCandidate>& cand)
{
  if (!activity.tp_list.empty()){
    std::cout << "\t\t\t\t\tCANDIDATE DETECTED" << std::endl;
    std::vector<uint16_t> detid_vector;
    detid_vector.push_back(activity.detid);

    std::vector<TriggerActivity> ta_list;
    ta_list.push_back(activity);
    
    TriggerCandidate tc {
      activity.time_start, 
      activity.time_end,  
      activity.time_start,
      detid_vector,
      TriggerCandidateType::kMichel,
      0,
      0,
      ta_list
    };
    cand.push_back(tc);
  }
}
