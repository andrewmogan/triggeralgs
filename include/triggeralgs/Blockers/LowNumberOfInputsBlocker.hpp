/**
 * @file LowNumberOfInputsBlocker.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_BLOCKERS_LOWNUMBEROFINPUTSBLOCKER_HPP_
#define TRIGGERALGS_BLOCKERS_LOWNUMBEROFINPUTSBLOCKER_HPP_

#include "triggeralgs/Blockers/BlockerDecorator.hpp"
#include <iostream>

namespace triggeralgs {

template <typename T>
class LowNumberOfInputsBlocker : public BlockerDecorator<T> {
  public:
    LowNumberOfInputsBlocker(std::shared_ptr<Blocker<T>> blocker) :
      BlockerDecorator<T>(blocker) {}

    bool block_logic(const T& input) const {
      return input.inputs.size() < m_min_inputs || BlockerDecorator<T>::block_logic(input);
    }

    void configure(const nlohmann::json& config) {
      m_min_inputs = 1;
      if (config.is_object() && config.contains("min_inputs")) {
        m_min_inputs = config["min_inputs"];
        std::cout << "\t\tSetting min_inputs to " << m_min_inputs << std::endl;
      }

      BlockerDecorator<T>::configure(config);
    }

  private:
    size_t m_min_inputs;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_BLOCKERS_LOWNUMBEROFINPUTSBLOCKER_HPP_
