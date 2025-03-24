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
#include "triggeralgs/Triton/Span.hpp"
#include "triggeralgs/Triton/triton_utils.hpp"
#include "triggeralgs/Triton/TritonData.hpp"
#include "triggeralgs/Triton/TritonClient.hpp"
#include "triggeralgs/Triton/TritonIssues.hpp"
#include "grpc_client.h"
#include "triggeralgs/Triton/json_utils.h"
//#include "ers/Issue.hpp"

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

namespace tc = triton::client;

ERS_DECLARE_ISSUE(
  triggeralgs,
  ModelNotReady,
  "Can't get readiness for model: " << model_name,
  ((std::string)model_name)
)

namespace triggeralgs {
class TriggerActivityMakerTriton : public TriggerActivityMaker
{
  public:
    void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas);
    void configure(const nlohmann::json& config);
    //void fail_if_error(const tc::Error& err, const std::string& msg) const;
    //bool warn_if_error(const tc::Error& err, const std::string& msg) const;
    void dump_config() const;
    std::vector<std::vector<std::vector<int>>> get_adcs_from_trigger_primitives(
      const uint64_t number_tps, 
      const uint64_t time_ticks, 
      const uint64_t number_wires);
    template <typename T> std::string print_collection(triton_span::Span<const T> coll, const std::string& delim);

  private:
    //std::unique_ptr<triton::client::InferenceServerGrpcClient> client;
    std::unique_ptr<triggeralgs::TritonClient> triton_client;
    uint64_t m_number_tps_per_request = 100;
    uint64_t m_number_time_ticks = 128;
    uint64_t m_number_wires = 128;
    uint64_t m_batch_size = 1;
    std::vector<std::string> m_outputs;
    bool m_print_tp_info = false;
    TriggerActivity m_current_ta;
    /* Triton config params should now be taken care of in TritonClient class*/
    //std::string m_inference_url = "localhost:8001";
    //std::string m_model_name = "simple";
    // The model version is a number representing a directory, so it's declared as a string here
    //std::string m_model_version = "1";
    //uint64_t m_timeout_microseconds = 5000;
    //unsigned m_triton_allowed_tries = 0;
};

} // namespace triggeralgs

#endif // TRIGGERALGS_TRITON_TRIGGERACTIVITYMAKERTRITON_HPP_
