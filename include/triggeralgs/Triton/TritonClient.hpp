#ifndef TRIGGERALGS_INCLUDE_TRITON_TRITONCLIENT_HPP
#define TRIGGERALGS_INCLUDE_TRITON_TRITONCLIENT_HPP

#include "triggeralgs/Triton/TritonData.hpp"

#include <memory>
#include <string>
#include <vector>

#include "grpc_client.h"
#include <nlohmann/json.hpp>

namespace tc = triton::client;

namespace triggeralgs {

  class TritonClient {
  public:
    struct ServerSideStats {
      uint64_t inference_count_;
      uint64_t execution_count_;
      uint64_t success_count_;
      uint64_t cumm_time_ns_;
      uint64_t queue_time_ns_;
      uint64_t compute_input_time_ns_;
      uint64_t compute_infer_time_ns_;
      uint64_t compute_output_time_ns_;
    };

    //constructor
    TritonClient(const nlohmann::json& client_config);
    ~TritonClient();

    void dump_config();

    //accessors
    TritonInputMap& input() { return input_; }
    const TritonOutputMap& output() const { return output_; }
    unsigned get_batch_size() const { return batch_size_; }
    bool verbose() const { return verbose_; }
    bool set_batch_size(unsigned bsize);

    const std::string& get_model_name() const {return options_.model_name_;}

    //main operation
    void dispatch()
    {
      start();
      evaluate();
    }

    //helper
    void reset();

  protected:
    //helper
    bool getResults(std::shared_ptr<tc::InferResult> results);

    void start();
    void evaluate();
    void finish(bool success);

    void reportServerSideStats(const ServerSideStats& stats) const;
    ServerSideStats summarizeServerStats(const inference::ModelStatistics& start_status,
                                         const inference::ModelStatistics& end_status) const;

    inference::ModelStatistics getServerSideStatus() const;

    //members
    TritonInputMap input_;
    TritonOutputMap output_;
    unsigned allowed_tries_, tries_;
    std::string inference_url_;
    unsigned maxBatchSize_;
    unsigned batch_size_;
    bool noBatch_;
    bool verbose_;
    bool ssl_;
    std::string sslRootCertificates_;
    std::string sslPrivateKey_;
    std::string sslCertificateChain_;
    // For reporting stats
    uint64_t first_inference_count_;
    uint64_t last_inference_count_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    std::chrono::time_point<std::chrono::steady_clock> end_time_;
    std::vector<uint64_t> request_latencies_;

    //IO pointers for triton
    std::vector<tc::InferInput*> inputsTriton_;
    std::vector<const tc::InferRequestedOutput*> outputsTriton_;

    std::unique_ptr<tc::InferenceServerGrpcClient> client_;
    //stores timeout, model name and version
    tc::InferOptions options_;
  };

}
#endif
