/**
 * @file TriggerCandidate.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATEPDS_HPP_
#define TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATEPDS_HPP_

#include "trgdataformats/TriggerActivityData.hpp"
#include "trgdataformats/TriggerCandidateData.hpp"

#include <vector>

namespace triggeralgs {

struct TriggerCandidatePDS : public dunedaq::trgdataformats::TriggerCandidateData
{
  std::vector<dunedaq::trgdataformats::TriggerActivityData > inputs;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERCANDIDATEPDS_HPP_
