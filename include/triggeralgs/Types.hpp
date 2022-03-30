/**
 * @file Types.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_INCLUDE_TRIGGERALGS_TYPES_HPP_
#define TRIGGERALGS_INCLUDE_TRIGGERALGS_TYPES_HPP_

#include "detdataformats/trigger/Types.hpp"

namespace triggeralgs {

using timestamp_t = dunedaq::detdataformats::trigger::timestamp_t;
using timestamp_diff_t = dunedaq::detdataformats::trigger::timestamp_diff_t;
using detid_t = dunedaq::detdataformats::trigger::detid_t;
using trigger_number_t = dunedaq::detdataformats::trigger::trigger_number_t;
using channel_t = dunedaq::detdataformats::trigger::channel_t;
using version_t = dunedaq::detdataformats::trigger::version_t;

} // namespace triggeralgs

#endif // TRIGGERALGS_INCLUDE_TRIGGERALGS_TYPES_HPP_
