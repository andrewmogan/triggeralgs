/**
 * @file Blocker.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_BLOCKERS_BLOCKER_HPP_
#define TRIGGERALGS_BLOCKERS_BLOCKER_HPP_

#include <nlohmann/json.hpp>
#include <memory>

namespace triggeralgs {

/*
 * The Blocker class is use to define the logics that will block a
 * TA/TC from being written. This class is intended to be decorated
 * on.
 */
template <typename T>
class Blocker {
  public:
    virtual ~Blocker() {}
    virtual bool block_logic(const T& input) const = 0;
    virtual void configure(const nlohmann::json& config) = 0;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_BLOCKERS_BLOCKER_HPP_
