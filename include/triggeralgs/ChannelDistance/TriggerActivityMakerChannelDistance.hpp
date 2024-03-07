/**
 * @file TriggerActivityMakerChannelDistance.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you shuold have
 * received with this code.
 */

#ifndef TRIGGERALGS_CHANNELDISTANCE_TRIGGERACTIVITYMAKERCHANNELDISTANCE_HPP_
#define TRIGGERALGS_CHANNELDISTANCE_TRIGGERACTIVITYMAKERCHANNELDISTANCE_HPP_

#include "triggeralgs/TriggerActivityFactory.hpp"

#include "triggeralgs/Blockers/SimpleBlocker.hpp"
#include "triggeralgs/Blockers/LowNumberOfInputsBlocker.hpp"

#include "triggeralgs/Closers/SimpleCloser.hpp"
#include "triggeralgs/Closers/MaxTimeCloser.hpp"

#include "triggeralgs/Skippers/SimpleSkipper.hpp"
#include "triggeralgs/Skippers/FarChannelSkipper.hpp"
#include "triggeralgs/Skippers/HighTimeOverThresholdSkipper.hpp"

namespace triggeralgs {

class TriggerActivityMakerChannelDistance : public TriggerActivityMaker {
  public:
    void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas);
    void configure(const nlohmann::json& config);
    void set_ta_attributes();

  private:
    std::shared_ptr<Blocker<TriggerActivity>> m_blocker
      = std::make_shared<LowNumberOfInputsBlocker<TriggerActivity>>();
    std::shared_ptr<Closer<TriggerPrimitive, TriggerActivity>> m_closer
      = std::make_shared<MaxTimeCloser>();
    std::shared_ptr<Skipper<TriggerPrimitive, TriggerActivity>> m_skipper
      = std::make_shared<FarChannelSkipper>(std::make_shared<HighTimeOverThresholdSkipper>());
    TriggerActivity m_current_ta;
};

}

#endif // TRIGGERALGS_CHANNELDISTANCE_TRIGGERACTIVITYMAKERCHANNELDISTANCE_HPP_
