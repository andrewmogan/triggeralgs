/**
 * @file BlockerDecorator.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_BLOCKERS_BLOCKERDECORATOR_HPP_
#define TRIGGERALGS_BLOCKERS_BLOCKERDECORATOR_HPP_

#include "triggeralgs/Blockers/Blocker.hpp"
#include "triggeralgs/Blockers/SimpleBlocker.hpp"

namespace triggeralgs {

/*
 * The BlockerDecorator decorates a Blocker with new block_logic conditions
 * and any new configurations.
 */
template <typename T>
class BlockerDecorator : public Blocker<T> {
  protected:
    std::shared_ptr<Blocker<T>> m_blocker;

  public:
    BlockerDecorator(std::shared_ptr<Blocker<T>> blocker) : m_blocker(blocker) {}
    BlockerDecorator() : m_blocker(std::make_shared<SimpleBlocker<T>>()) {}

    bool block_logic(const T& input) const {
      return this->m_blocker->block_logic(input);
    }

    void configure(const nlohmann::json& config) {
      this->m_blocker->configure(config);
    }
};

} // namespace triggeralgs

#endif // TRIGGERALGS_BLOCKERS_BLOCKERDECORATOR_HPP_
