/**
 * @file FarChannelSkipper.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SKIPPERS_FARCHANNELSKIPPER_HPP_
#define TRIGGERALGS_SKIPPERS_FARCHANNELSKIPPER_HPP_

#include "triggeralgs/Skippers/SkipperDecorator.hpp"
#include "triggeralgs/TriggerActivity.hpp"
#include "triggeralgs/TriggerPrimitive.hpp"
#include "triggeralgs/Types.hpp"

#include <cmath>

namespace triggeralgs {

/*
 * Skip over any TPs that are far from the processed TPs.
 */
class FarChannelSkipper : public SkipperDecorator<TriggerPrimitive, TriggerActivity> {
  public:
    FarChannelSkipper(std::shared_ptr<Skipper<TriggerPrimitive, TriggerActivity>> skipper) :
      SkipperDecorator<TriggerPrimitive, TriggerActivity>(skipper) {}
    FarChannelSkipper() : SkipperDecorator<TriggerPrimitive, TriggerActivity>() {}

    bool skip_logic(const TriggerPrimitive& new_tp, const TriggerActivity& ta) const {
      bool far_tp = false;
      for (const TriggerPrimitive& tp : ta.inputs) {
        if (std::abs(tp.channel - new_tp.channel) > m_max_channel_distance) {
          far_tp = true;
          break;
        }
      }
      return far_tp || SkipperDecorator<TriggerPrimitive, TriggerActivity>::skip_logic(new_tp, ta);
    }

    void configure(const nlohmann::json& config) {
      m_max_channel_distance = 5; // TODO: Arbitrary distance.
      if (config.is_object() && config.contains("max_channel_distance")) {
        m_max_channel_distance = config["max_channel_distance"];
      }

      SkipperDecorator<TriggerPrimitive, TriggerActivity>::configure(config);
    }

  private:
    channel_t m_max_channel_distance;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_SKIPPERS_FARCHANNELSKIPPER_HPP_
