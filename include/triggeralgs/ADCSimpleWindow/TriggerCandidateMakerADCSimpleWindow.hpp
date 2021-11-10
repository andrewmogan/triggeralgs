/**
 * @file TriggerCandidateMakerADCSimpleWindow.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_ADCSIMPLEWINDOW_TRIGGERCANDIDATEMAKERADCSIMPLEWINDOW_HPP_
#define TRIGGERALGS_ADCSIMPLEWINDOW_TRIGGERCANDIDATEMAKERADCSIMPLEWINDOW_HPP_

#include "triggeralgs/TriggerCandidateMaker.hpp"

#include <vector>

namespace triggeralgs {
class TriggerCandidateMakerADCSimpleWindow : public TriggerCandidateMaker
{

public:
  /// The function that gets call when there is a new activity
  void operator()(const TriggerActivity&, std::vector<TriggerCandidate>&);
  
  void configure(const nlohmann::json &config);
  
private:

  uint64_t m_activity_count = 0; // NOLINT(build/unsigned)
  
};

} // namespace triggeralgs

#endif // TRIGGERALGS_ADCSIMPLEWINDOW_TRIGGERCANDIDATEMAKERADCSIMPLEWINDOW_HPP_
