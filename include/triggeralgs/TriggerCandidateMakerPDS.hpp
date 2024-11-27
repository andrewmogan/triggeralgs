/**
 * @file TriggerCandidateMaker.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATEMAKERPDS_HPP_
#define TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATEMAKERPDS_HPP_

#include "triggeralgs/Issues.hpp"
#include "triggeralgs/Logging.hpp"
#include "triggeralgs/TriggerActivityPDS.hpp"
#include "triggeralgs/TriggerCandidatePDS.hpp"
#include "triggeralgs/Types.hpp"

#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>
#include <vector>

namespace triggeralgs {

class TriggerCandidateMakerPDS
{
public:
  virtual ~TriggerCandidateMakerPDS() = default;
  virtual void operator()(const TriggerActivityPDS& input_ta, std::vector<TriggerCandidatePDS>& output_tc) = 0;
  virtual void flush(timestamp_t /* until */, std::vector<TriggerCandidatePDS>& /* output_tc */) {}
  virtual void configure(const nlohmann::json&) {}
};

} // namespace triggeralgs

#endif // TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATEMAKER_HPP_
