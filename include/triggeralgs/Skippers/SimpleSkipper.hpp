/**
 * @file SimpleSkipper.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SKIPPERS_SIMPLESKIPPER_HPP_
#define TRIGGERALGS_SKIPPERS_SIMPLESKIPPER_HPP_

#include "triggeralgs/Skippers/Skipper.hpp"

namespace triggeralgs {

/*
 * This is a simple skipper that decorator skippers can
 * decorate on. This skipper will skip everything on its
 * own and configures nothing.
 */
template <typename T, typename U>
class SimpleSkipper : public Skipper<T, U> {
  public:
    bool skip_logic(const T& tx, const U& ty) const {
      return false;
    }

    void configure(const nlohmann::json& config) {
      return;
    }
};

} // namespace triggeralgs

#endif // TRIGGERALGS_SKIPPERS_SIMPLESKIPPER_HPP_
