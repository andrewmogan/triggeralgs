/**
 * @file TriggerCandidateType.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATETYPE_HPP_
#define TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATETYPE_HPP_

namespace triggeralgs {
enum class TriggerCandidateType
{
  kUnknown = 0,
  kTiming = 1,
  kTPCLowE = 2,
  kSupernova = 3,
  kRandom = 4,
  kPrescale = 5,
  kMichel = 6
};
} // namespace triggeralgs

#endif // TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATETYPE_HPP_
