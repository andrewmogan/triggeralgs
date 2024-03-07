/**
 * @file MaxTimeCloser.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_CLOSERS_MAXTIMECLOSER_HPP_
#define TRIGGERALGS_CLOSERS_MAXTIMECLOSER_HPP_

#include "triggeralgs/Closers/CloserDecorator.hpp"
#include "triggeralgs/TriggerActivity.hpp"
#include "triggeralgs/TriggerPrimitive.hpp"
#include "triggeralgs/Types.hpp"

#include <iostream>

namespace triggeralgs {

/*
 * Close on any TP that is far in time from the first TP.
 */
class MaxTimeCloser : public CloserDecorator<TriggerPrimitive, TriggerActivity> {
  public:
    MaxTimeCloser(std::shared_ptr<Closer<TriggerPrimitive, TriggerActivity>> closer) :
      CloserDecorator<TriggerPrimitive, TriggerActivity>(closer) {}
    MaxTimeCloser() : CloserDecorator<TriggerPrimitive, TriggerActivity>() {}

    bool close_logic(const TriggerPrimitive& tp, const TriggerActivity& ta) const {
      timestamp_t ta_start = ta.inputs.front().time_start; // Assumes ta is populated.
      std::cout << "\t\tTime Difference = " << tp.time_start - ta_start << std::endl;
      return tp.time_start - ta_start > m_max_time_delta || CloserDecorator<TriggerPrimitive, TriggerActivity>::close_logic(tp, ta);
    }

    void configure(const nlohmann::json& config) {
      if (config.is_object() && config.contains("max_time_delta")) {
        m_max_time_delta = config["max_time_delta"];
      }

      CloserDecorator<TriggerPrimitive, TriggerActivity>::configure(config);
    }

  private:
    timestamp_t m_max_time_delta;
};
} // namespace triggeralgs

#endif // TRIGGERALGS_CLOSERS_MAXTIMECLOSER_HPP_
