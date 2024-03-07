/**
 * @file SkipperDecorator.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SKIPPERS_SKIPPERDECORATOR_HPP_
#define TRIGGERALGS_SKIPPERS_SKIPPERDECORATOR_HPP_

#include "triggeralgs/Skippers/Skipper.hpp"
#include "triggeralgs/Skippers/SimpleSkipper.hpp"

namespace triggeralgs {

/*
 * The SkipperDecorator decorates a Skipper with new skip_logic conditions
 * and any new configurations.
 */
template <typename T, typename U>
class SkipperDecorator : public Skipper<T, U> {
  protected:
    std::shared_ptr<Skipper<T, U>> m_skipper;

  public:
    SkipperDecorator(std::shared_ptr<Skipper<T, U>> skipper) : m_skipper(skipper) {}
    SkipperDecorator() : m_skipper(std::make_shared<SimpleSkipper<T, U>>()) {}

    bool skip_logic(const T& tx, const U& ty) const {
      return this->m_skipper->skip_logic(tx, ty);
    }

    void configure(const nlohmann::json& config) {
      this->m_skipper->configure(config);
    }
};

} // namespace triggeralgs

#endif // TRIGGERALGS_SKIPPERS_SKIPPERDECORATOR_HPP_
