#include "triggeralgs/Triton/triton_utils.hpp"
#include "triggeralgs/Triton/TritonIssues.hpp"
#include "triggeralgs/Triton/Span.hpp"
#include "TRACE/trace.h"

#include <algorithm>
#include <experimental/iterator>
#include <iostream>
#include <sstream>
#include <string>

namespace triggeralgs{
namespace triton_utils {

  template <typename C>
  std::string print_collection(const C& coll, const std::string& delim)
  {
    if (coll.empty()) return "";
    using std::begin;
    using std::end;
    std::stringstream msg;
    //avoid trailing delim
    std::copy(begin(coll), end(coll), std::experimental::make_ostream_joiner(msg, delim));
    return msg.str();
  }

  void fail_if_error(const tc::Error& err, const std::string& msg) {
    if (!err.IsOk()) {
      TLOG() << "[TA:Triton] ERROR: " << msg << ": " << err << std::endl;
    exit(1);
    }
  }

  bool warn_if_error(const tc::Error& err, const std::string& msg) {
    if (!err.IsOk()) {
      TLOG() << "[TA:Triton] WARNING: " << msg << ": " << err << std::endl;
    }
    return err.IsOk();
  }
} // namespace triton_utils
} // namespace triggeralgs

template std::string triggeralgs::triton_utils::print_collection(
  const triton_span::Span<std::vector<int64_t>::const_iterator>& coll,
  const std::string& delim);
template std::string triggeralgs::triton_utils::print_collection(
  const std::vector<uint8_t>& coll,const std::string& delim);
template std::string triggeralgs::triton_utils::print_collection(
  const std::vector<float>& coll,const std::string& delim);
template std::string triggeralgs::triton_utils::print_collection(
  const std::unordered_set<std::string>& coll, const std::string& delim);
