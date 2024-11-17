/**
 * @file TriggerActivityPDS.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERACTIVITYPDS_HPP_
#define TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERACTIVITYPDS_HPP_

#include "trgdataformats/TriggerActivityData.hpp"
#include "triggeralgs/TriggerPrimitivePDS.hpp"

#include <vector>

namespace triggeralgs {

struct TriggerActivityPDS : public dunedaq::trgdataformats::TriggerActivityData
{
  std::vector<TriggerPrimitivePDS> inputs;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_INCLUDE_TRIGGERALGS_TRIGGERACTIVITY_HPP_
