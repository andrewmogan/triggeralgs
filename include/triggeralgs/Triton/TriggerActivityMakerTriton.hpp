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
#include "grpc_client.h"
#include "triggeralgs/Triton/json_utils.h"
#include "ers/Issue.hpp"

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

namespace tc = triton::client;
namespace triggeralgs {

ERS_DECLARE_ISSUE(
  triggeralgs,
  ServerNotLive,
  "Could not get server liveness"
)

class TriggerActivityMakerTriton : public TriggerActivityMaker
{
  public:
    void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas);
    void configure(const nlohmann::json& config);
    void fail_if_error(const tc::Error& err, const std::string& msg) const;
    void dump_config() const;
    void check_triton_server_liveness(const std::string& inference_url) const;
    //void check_model_readiness(const std::string model_name, const std::string model_version) const;
    void check_model_readiness(const std::string model_name, const std::string model_version) const;
    void check_model_inputs(const std::string model_name, const std::string model_version) const;
    void ValidateSimpleShapeAndDatatype(const std::string& name, std::shared_ptr<tc::InferResult> result) const;
    void ValidateSimpleResult(
      const std::shared_ptr<tc::InferResult> result, 
      std::vector<int32_t>& input0_data, 
      std::vector<int32_t>& input1_data) const;
    //void query_triton_server(const TriggerActivity& trigger_activity, const std::string& inference_url);

  private:
    uint64_t m_number_tps_per_request = 100;
    std::string m_inference_url = "localhost:8001";
    std::string m_model_name = "simple";
    // The model version is a number representing a directory, so it's declared as a string here
    std::string m_model_version = "1";
    uint64_t m_client_timeout_microseconds = 5000;
    uint64_t m_server_timeout_microseconds = 5000;
    bool m_print_tp_info = false;
    TriggerActivity m_current_ta;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_TRITON_TRIGGERACTIVITYMAKERTRITON_HPP_
