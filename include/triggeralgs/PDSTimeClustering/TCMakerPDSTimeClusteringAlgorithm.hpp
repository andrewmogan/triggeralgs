/**
 * @file TCMakerADCSimpleWindowAlgorithm.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_PDSTIMECLUSTERING_TRIGGERCANDIDATEMAKERPDSTIMECLUSTERING_HPP_
#define TRIGGERALGS_PDSTIMECLUSTERING_TRIGGERCANDIDATEMAKERPDSTIMECLUSTERING_HPP_

#include "triggeralgs/TriggerCandidateFactoryPDS.hpp"

#include <vector>

namespace triggeralgs {
class TCMakerPDSTimeClusteringAlgorithm : public TriggerCandidateMakerPDS
{

public:
  /// The function that gets call when there is a new activity
  void process(const TriggerActivityPDS&, std::vector<TriggerCandidatePDS>&);
  
  void configure(const nlohmann::json &config);
  
private:

  uint64_t m_activity_count = 0; // NOLINT(build/unsigned)
  
};

} // namespace triggeralgs

#endif // TRIGGERALGS_PDSTIMECLUSTERING_TRIGGERCANDIDATEMAKERPDSTIMECLUSTERING_HPP_
