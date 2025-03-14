#ifndef TRIGGERALGS_TRITON_TRITON_UTILS_HPP
#define TRIGGERALGS_TRITON_TRITON_UTILS_HPP

#include <string>
#include <string_view>

#include "grpc_client.h"

namespace tc = triton::client;

namespace triggeralgs{
namespace triton_utils {

  template <typename C>
  std::string print_collection(const C& coll, const std::string& delim = ", ");

  void fail_if_error(const tc::Error& err, const std::string& msg);
  bool warn_if_error(const tc::Error& err, const std::string& msg);

} // namespace triton_utils
} // namespace triggeralgs

#endif
