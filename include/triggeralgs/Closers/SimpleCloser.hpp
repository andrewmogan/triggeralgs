/**
 * @file SimpleCloser.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_CLOSERS_SIMPLECLOSER_HPP_
#define TRIGGERALGS_CLOSERS_SIMPLECLOSER_HPP_

#include "triggeralgs/Closers/Closer.hpp"

namespace triggeralgs {

/*
 * This is a simple closer that decorator closers can
 * decorate on. This closer will close everything on its
 * own and configures nothing.
 */
template <typename T, typename U>
class SimpleCloser : public Closer<T, U> {
  public:
    bool close_logic(const T& tx, const U& ty) const {
      return false;
    }

    void configure(const nlohmann::json& config) {
      return;
    }
};
} // namespace triggeralgs

#endif // TRIGGERALGS_CLOSERS_SIMPLECLOSER_HPP_
