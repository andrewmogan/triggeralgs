/**
 * @file HighTimeOverThresholdSkipper.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SKIPPERS_HIGHTIMEOVERTHRESHOLDSKIPPER_HPP_
#define TRIGGERALGS_SKIPPERS_HIGHTIMEOVERTHRESHOLDSKIPPER_HPP_

#include "triggeralgs/Skippers/SkipperDecorator.hpp"
#include "triggeralgs/TriggerActivity.hpp"
#include "triggeralgs/TriggerPrimitive.hpp"
#include "triggeralgs/Types.hpp"

namespace triggeralgs {

/*
 * Skip over any TPs that have a high time_over_threshold.
 */
class HighTimeOverThresholdSkipper : public SkipperDecorator<TriggerPrimitive, TriggerActivity> {
  public:
    HighTimeOverThresholdSkipper(std::shared_ptr<Skipper<TriggerPrimitive, TriggerActivity>> skipper) :
      SkipperDecorator<TriggerPrimitive, TriggerActivity>(skipper) {}
    HighTimeOverThresholdSkipper() : SkipperDecorator<TriggerPrimitive, TriggerActivity>() {}

    // skip_logic does not need ta in this case, but needs to pass it on.
    bool skip_logic(const TriggerPrimitive& tp, const TriggerActivity& ta) const {
      return tp.time_over_threshold > m_tot_limit
        || SkipperDecorator<TriggerPrimitive, TriggerActivity>::skip_logic(tp, ta);
    }

    void configure(const nlohmann::json& config) {
      m_tot_limit = 1'000'000; // TODO: Arbitrary time.
      if (config.is_object() && config.contains("tot_limit"))
        m_tot_limit = config["tot_limit"];

      SkipperDecorator::configure(config);
    }

  private:
    timestamp_t m_tot_limit;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_SKIPPERS_HIGHTIMEOVERTHRESHOLDSKIPPER_HPP_
