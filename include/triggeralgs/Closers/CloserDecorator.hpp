/**
 * @file CloserDecorator.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_CLOSERS_CLOSERDECORATOR_HPP_
#define TRIGGERALGS_CLOSERS_CLOSERDECORATOR_HPP_

#include "triggeralgs/Closers/Closer.hpp"

namespace triggeralgs {

/*
 * The CloserDecorator decorates a Closer with new close_logic conditions
 * and any new configurations.
 */
template <typename T, typename U>
class CloserDecorator : public Closer<T, U> {
  protected:
    std::shared_ptr<Closer<T, U>> m_closer;

  public:
    CloserDecorator(std::shared_ptr<Closer<T,U>> closer) : m_closer(closer) {}

    bool close_logic(const T& tx, const U& ty) const {
      return this->m_closer->close_logic(tx, ty);
    }

    void configure(const nlohmann::json& config) {
      this->m_closer->configure(config);
    }
};

} // namespace triggeralgs

#endif // TRIGGERALGS_CLOSERS_CLOSERDECORATOR_HPP_

