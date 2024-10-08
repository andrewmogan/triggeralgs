/**
 * @file TritonIssues.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_TRITON_TRITONISSUES_HPP_
#define TRIGGERALGS_TRITON_TRITONISSUES_HPP_

#include "ers/Issue.hpp"

#include <string>

ERS_DECLARE_ISSUE(
  triggeralgs,
  ServerNotLive,
  "Could not get server liveness",
)
ERS_DECLARE_ISSUE(
  triggeralgs,
  CantGetServerMetadata,
  "Could not get server metadata",
)
ERS_DECLARE_ISSUE(
  triggeralgs,
  UnexpectedServerMetadata,
  "Unexpected server metadata: " << meta_string,
  ((std::string)meta_string)
)

#endif  // TRIGGERALGS_TRITON_TRITONISSUES_HPP_