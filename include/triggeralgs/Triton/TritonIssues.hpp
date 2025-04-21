/**
 * @file TritonIssues.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_INCLUDE_TRITON_TRITONISSUES_HPP_
#define TRIGGERALGS_INCLUDE_TRITON_TRITONISSUES_HPP_

#include "ers/Issue.hpp"
#include "logging/Logging.hpp"

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
// TODO: Add name_, data_in.size(), and batch_size_ to this message
/*
ERS_DECLARE_ISSUE(
    triggeralgs,
    TritonDataMismatch,
    "TritonData batch size mismatch: expected " << expected << ", got " << got,
    ((size_t)expected)
    ((size_t)got)
)
*/
// TODO: Add sizeof(DT), byte_size_, and dname_ to this message
ERS_DECLARE_ISSUE(
  triggeralgs,
  TritonDataByteSizeError,
  "Inconsistent byte size",
  //<< name_ << " input(): inconsistent byte size " << sizeof(DT) << " (should be "
  //<< byteSize_ << " for " << dname_ << ")";
)

#endif  // TRIGGERALGS_INCLUDE_TRITON_TRITONISSUES_HPP_