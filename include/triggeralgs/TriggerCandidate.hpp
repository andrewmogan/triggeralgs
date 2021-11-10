/**
 * @file TriggerCandidate.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATE_HPP_
#define TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATE_HPP_

#include "triggeralgs/TriggerActivity.hpp"
#include "triggeralgs/Types.hpp"

#include <cstdint>
#include <vector>

namespace triggeralgs {
struct TriggerCandidate
{
  enum class Type {
    kUnknown = 0,
    kTiming = 1,
    kTPCLowE = 2,
    kSupernova = 3,
    kRandom = 4,
    kPrescale = 5,
    kADCSimpleWindow = 6,
  };

  enum class Algorithm {
    kUnknown = 0,
    kSupernova = 1,
    kHSIEventToTriggerCandidate = 2,
    kPrescale = 3,
    kADCSimpleWindow = 4
  };

  timestamp_t time_start = INVALID_TIMESTAMP;
  timestamp_t time_end = INVALID_TIMESTAMP;
  timestamp_t time_candidate = INVALID_TIMESTAMP;
  std::vector<detid_t> detid; // NOLINT(build/unsigned)
  Type type = Type::kUnknown;
  Algorithm algorithm = Algorithm::kUnknown; // NOLINT(build/unsigned)
  version_t version = INVALID_VERSION;   // NOLINT(build/unsigned)

  std::vector<TriggerActivity> ta_list;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATE_HPP_
