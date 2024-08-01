/**
 * @file TriggerActivityMakerTriton.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_TRITON_TRIGGERACTIVITYMAKERTRITON_HPP_
#define TRIGGERALGS_TRITON_TRIGGERACTIVITYMAKERTRITON_HPP_

#include "triggeralgs/TriggerActivityFactory.hpp"

//#include "grpc_client.h"

#include <vector>
#include <string>

namespace triggeralgs {

class TriggerActivityMakerTriton : public TriggerActivityMaker
{
  public:
    void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas);
    void configure(const nlohmann::json& config);
    void query_triton_server(const TriggerActivity& trigger_activity, const std::string& inference_url);

  private:
      uint64_t m_number_tps_per_request = 100;
      std::string m_inference_url = "localhost:8001";
      std::string m_model_name = "simple";
      // The model version is a number representing a directory, so it's declared as a string here
      std::string m_model_version = "1";
      bool m_print_tp_info = false;
      TriggerActivity m_current_ta;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_TRITON_TRIGGERACTIVITYMAKERTRITON_HPP_
