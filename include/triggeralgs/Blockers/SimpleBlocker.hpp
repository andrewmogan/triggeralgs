/**
 * @file SimpleBlocker.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_BLOCKERS_SIMPLEBLOCKER_HPP_
#define TRIGGERALGS_BLOCKERS_SIMPLEBLOCKER_HPP_

#include "triggeralgs/Blockers/Blocker.hpp"

namespace triggeralgs {

/*
 * This is a simple blocker that decorator blockers can
 * decorate on. This blocker will not block anything on
 * its own and configures nothing.
 */
template <typename T>
class SimpleBlocker : public Blocker<T> {
  public:
    bool block_logic(const T& input) const {
      return false;
    }

    void configure(const nlohmann::json& config) {
      return;
    }
};

} // namespace triggeralgs

#endif // TRIGGERALGS_BLOCKERS_SIMPLEBLOCKER_HPP_

