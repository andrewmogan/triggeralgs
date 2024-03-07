/**
 * @file Closer.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_CLOSERS_CLOSER_HPP_
#define TRIGGERALGS_CLOSERS_CLOSER_HPP_

#include <nlohmann/json.hpp>
#include <memory>

namespace triggeralgs {

/*
 * The Closer class is used to define the logics that will close a
 * TA/TC and move onto a Blocker. This class is intended to be decorated
 * on.
 */
template <typename T, typename U>
class Closer {
  public:
    virtual ~Closer() {}
    virtual bool close_logic(const T& tx, const U& ty) const = 0;
    virtual void configure(const nlohmann::json& config) = 0;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_CLOSERS_CLOSER_HPP_

