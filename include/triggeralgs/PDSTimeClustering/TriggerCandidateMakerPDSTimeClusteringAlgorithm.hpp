/**
 * @file TriggerCandidateMakerPDSTimeClusteringAlgorithm.hpp
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
class TriggerCandidateMakerPDSTimeClusteringAlgorithm : public TriggerCandidateMakerPDS
{

public:
  /// The function that gets call when there is a new activity
  void operator()(const TriggerActivityPDS&, std::vector<TriggerCandidatePDS>&);
  
  void configure(const nlohmann::json &config);
  
private:

  uint64_t m_activity_count = 0; // NOLINT(build/unsigned)

  /// @brief Configurable TC type to produce by this TC algorithm
  TriggerCandidatePDS::Type m_tc_type = TriggerCandidatePDS::Type::kPDSTimeClustering;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_PDSTIMECLUSTERING_TRIGGERCANDIDATEMAKERPDSTIMECLUSTERING_HPP_
