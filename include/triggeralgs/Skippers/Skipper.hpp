/**
 * @file Skipper.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SKIPPERS_SKIPPER_HPP_
#define TRIGGERALGS_SKIPPERS_SKIPPER_HPP_

#include <nlohmann/json.hpp>
#include <memory>

namespace triggeralgs {

/*
 * The Skipper class is used to define the logics that will skip a
 * TP/TA from being appended. This class is intended to be decorated
 * on.
 */
template <typename T, typename U>
class Skipper {
  public:
    virtual ~Skipper() {}
    virtual bool skip_logic(const T& tx, const U& ty) const = 0;
    virtual void configure(const nlohmann::json& config) = 0;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_SKIPPERS_SKIPPER_HPP_
