#include "TRACE/trace.h"
#include "triggeralgs/Triton/TritonClient.hpp"

#include "grpc_client.h"

#include <chrono>
#include <cmath>
#include <exception>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace tc = triton::client;

//based on https://github.com/triton-inference-server/server/blob/v2.3.0/src/clients/c++/examples/simple_grpc_async_infer_client.cc
//and https://github.com/triton-inference-server/server/blob/v2.3.0/src/clients/c++/perf_client/perf_client.cc

namespace triggeralgs {

  using triton_utils::fail_if_error;
  using triton_utils::warn_if_error;

  TritonClient::TritonClient(const nlohmann::json& client_config)
    : allowed_tries_(client_config.value("allowed_tries", 0))
    , inference_url_(client_config.at("inference_url"))
    , verbose_(client_config.value("verbose", 0))
    , ssl_(client_config.value("ssl", false))
    , sslRootCertificates_(client_config.value("sslRootCertificates", ""))
    , sslPrivateKey_(client_config.value("sslPrivateKey", ""))
    , sslCertificateChain_(client_config.value("sslCertificateChain", ""))
    , options_(client_config.at("model_name"))
  {
    if (verbose_) TLOG() << "Server URL: " << inference_url_;

    //connect to the server
    if (ssl_) {
      tc::SslOptions ssl_options = tc::SslOptions();
      ssl_options.root_certificates = sslRootCertificates_;
      ssl_options.private_key = sslPrivateKey_;
      ssl_options.certificate_chain = sslCertificateChain_;
      fail_if_error(
        tc::InferenceServerGrpcClient::Create(
          &client_, inference_url_, verbose_, true, ssl_options, tc::KeepAliveOptions(), true),
          "TritonClient(): unable to create inference context");
    }
    else {
      fail_if_error(
        tc::InferenceServerGrpcClient::Create(&client_, inference_url_, verbose_, false),
        "TritonClient(): unable to create inference context");
    }

    options_.model_version_ = client_config.at("model_version");
    options_.client_timeout_ = client_config.value("client_timeout_microseconds", 0);

    //config needed for batch size
    inference::ModelConfigResponse modelConfigResponse;
    fail_if_error(
      client_->ModelConfig(&modelConfigResponse, options_.model_name_, options_.model_version_),
      "TritonClient(): unable to get model config");
    inference::ModelConfig modelConfig(modelConfigResponse.config());

    //check batch size limitations (after i/o setup)
    //triton uses max batch size = 0 to denote a model that does not support batching
    //but for models that do support batching, a given event may set batch size 0 to indicate no valid input is present
    //so set the local max to 1 and keep track of "no batch" case
    maxBatchSize_ = modelConfig.max_batch_size();
    noBatch_ = maxBatchSize_ == 0;
    maxBatchSize_ = std::max(1u, maxBatchSize_);

    inference::ModelMetadataResponse modelMetadata;
    fail_if_error(
      client_->ModelMetadata(&modelMetadata, options_.model_name_, options_.model_version_),
      "TritonClient(): unable to get model metadata");

    const auto& nicInputs = modelMetadata.inputs();
    const auto& nicOutputs = modelMetadata.outputs();

    //report all model errors at once
    std::ostringstream msg;
    std::string msg_str;

    //currently no use case is foreseen for a model with zero inputs or outputs
    if (nicInputs.empty()) msg << "Model on server appears malformed (zero inputs)\n";

    if (nicOutputs.empty()) msg << "Model on server appears malformed (zero outputs)\n";

    //stop if errors
    msg_str = msg.str();
    if (!msg_str.empty()) throw triggeralgs::CantGetServerMetadata(ERS_HERE);

    //setup input map
    std::ostringstream io_msg;
    if (verbose_)
      io_msg << "Model inputs: "
             << "\n";
    inputsTriton_.reserve(nicInputs.size());
    for (const auto& nicInput : nicInputs) {
      const auto& iname = nicInput.name();
      auto [curr_itr, success] = input_.try_emplace(iname, iname, nicInput, noBatch_);
      auto& curr_input = curr_itr->second;
      inputsTriton_.push_back(curr_input.data());
      if (verbose_) {
        io_msg << "  " << iname << " (" << curr_input.get_dname() << ", " << curr_input.get_byte_size()
               << " b) : " << triton_utils::print_collection(curr_input.get_shape()) << "\n";
      }
    }

    //allow selecting only some outputs from server
    const auto& v_outputs = client_config.value("outputs", std::vector<std::string>{});
    std::unordered_set<std::string> s_outputs(v_outputs.begin(), v_outputs.end());

    //setup output map
    if (verbose_)
      io_msg << "Model outputs: "
             << "\n";
    outputsTriton_.reserve(nicOutputs.size());
    for (const auto& nicOutput : nicOutputs) {
      const auto& oname = nicOutput.name();
      if (!s_outputs.empty() and s_outputs.find(oname) == s_outputs.end()) continue;
      auto [curr_itr, success] = output_.try_emplace(oname, oname, nicOutput, noBatch_);
      auto& curr_output = curr_itr->second;
      outputsTriton_.push_back(curr_output.data());
      if (verbose_) {
        io_msg << "  " << oname << " (" << curr_output.get_dname() << ", " << curr_output.get_byte_size()
               << " b) : " << triton_utils::print_collection(curr_output.get_shape()) << "\n";
      }
      if (!s_outputs.empty()) s_outputs.erase(oname);
    }

    if (!s_outputs.empty())
      // TODO Add this as an exception
      //throw cet::exception("MissingOutput")
      TLOG() << "MissingOutput:"
        << "Some requested outputs were not available on the server: "
        << triton_utils::print_collection(s_outputs);

    //propagate batch size to inputs and outputs
    set_batch_size(1);

    //print model info
    if (verbose_) {
      std::ostringstream model_msg;
      model_msg << "Model name: " << options_.model_name_ << "\n"
                << "Model version: " << options_.model_version_ << "\n"
                << "Model max batch size: " << (noBatch_ ? 0 : maxBatchSize_) << "\n";
      TLOG() << model_msg.str() << io_msg.str();
    }

    first_inference_count_ = 0;
  }

  TritonClient::~TritonClient() {
    TLOG() << "Delta inference count: " << last_inference_count_ << ", " << first_inference_count_;
    double seconds = std::chrono::duration<double>(end_time_ - start_time_).count();
    TLOG() << "Seconds: " << seconds;
    double throughput = (last_inference_count_ - first_inference_count_) / seconds;
    TLOG() << "\n\tAverage throughput: " << throughput << " inferences/second";
  }

  bool TritonClient::set_batch_size(unsigned bsize)
  {
    if (bsize > maxBatchSize_) {
      //MF_LOG_WARNING("TritonClient")
      TLOG() << "Requested batch size " << bsize << " exceeds server-specified max batch size "
             << maxBatchSize_ << ". Batch size will remain as" << batch_size_;
      return false;
    }
    batch_size_ = bsize;
    //set for input and output
    for (auto& element : input_) {
      element.second.set_batch_size(bsize);
    }
    for (auto& element : output_) {
      element.second.set_batch_size(bsize);
    }
    return true;
  }

  void TritonClient::reset()
  {
    for (auto& element : input_) {
      element.second.reset();
    }
    for (auto& element : output_) {
      element.second.reset();
    }
  }

  bool TritonClient::getResults(std::shared_ptr<tc::InferResult> results)
  {
    for (auto& [oname, output] : output_) {
      //set shape here before output becomes const
      if (output.variable_dims()) {
        std::vector<int64_t> tmp_shape;
        bool status =
          warn_if_error(results->Shape(oname, &tmp_shape),
                        "getResults(): unable to get output shape for " + oname);
        if (!status) return status;
        output.set_shape(tmp_shape, false);
      }
      //extend lifetime
      output.set_result(results);
    }

    return true;
  }

  void TritonClient::start()
  {
    tries_ = 0;
  }

  //default case for sync and pseudo async
  void TritonClient::evaluate()
  {
    //in case there is nothing to process
    if (batch_size_ == 0) {
      finish(true);
      return;
    }

    const auto& start_status = getServerSideStatus();

    // Blocking call
    auto t1 = std::chrono::steady_clock::now();
    //if (start_time_ == 0) start_time_ = t1;
    if (first_inference_count_ == 0) {
      first_inference_count_ = start_status.inference_count();
      start_time_ = t1;
    }
    tc::InferResult* results;

    tc::Headers http_headers;
    grpc_compression_algorithm compression_algorithm =
      grpc_compression_algorithm::GRPC_COMPRESS_NONE;

    bool status = warn_if_error(
      client_->Infer(
        &results, options_, inputsTriton_, outputsTriton_, http_headers, compression_algorithm),
      "evaluate(): unable to run and/or get result");
    if (!status) {
      finish(false);
      return;
    }

    auto t2 = std::chrono::steady_clock::now();
    TLOG() << "\n\tRemote time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    end_time_ = t2;

    const auto& end_status = getServerSideStatus();
    last_inference_count_ = end_status.inference_count();

    if (verbose()) {
      const auto& stats = summarizeServerStats(start_status, end_status);
      reportServerSideStats(stats);
    }

    std::shared_ptr<tc::InferResult> results_ptr(results);
    status = getResults(results_ptr);

    finish(status);
  }

  void TritonClient::finish(bool success)
  {
    if (!success) {
      ++tries_;
      if (tries_ < allowed_tries_) {
        evaluate();
        // Avoid calling doneWaiting() twice
        return;
      }
      // Prepare an exception if exceeded
      // TODO Make this an exception
      //throw cet::exception("TritonClient")
      TLOG() << "THIS SHOULD BE AN ERROR"
        << "call failed after max " << tries_ << " tries" << std::endl;
    }
  }

  void TritonClient::reportServerSideStats(const TritonClient::ServerSideStats& stats) const
  {
    std::ostringstream msg;

    // https://github.com/triton-inference-server/server/blob/v2.3.0/src/clients/c++/perf_client/inference_profiler.cc
    const uint64_t count = stats.success_count_;
    msg << "\n\tInference count: " << stats.inference_count_ << "\n";
    msg << "\n\tExecution count: " << stats.execution_count_ << "\n";
    msg << "\n\tSuccessful request count: " << count << "\n";

    if (count > 0) {
      auto get_avg_us = [count](uint64_t tval) {
        constexpr uint64_t us_to_ns = 1000;
        return tval / us_to_ns / count;
      };

      const uint64_t cumm_avg_us = get_avg_us(stats.cumm_time_ns_);
      const uint64_t queue_avg_us = get_avg_us(stats.queue_time_ns_);
      const uint64_t compute_input_avg_us = get_avg_us(stats.compute_input_time_ns_);
      const uint64_t compute_infer_avg_us = get_avg_us(stats.compute_infer_time_ns_);
      const uint64_t compute_output_avg_us = get_avg_us(stats.compute_output_time_ns_);
      const uint64_t compute_avg_us =
        compute_input_avg_us + compute_infer_avg_us + compute_output_avg_us;
      const uint64_t overhead = (cumm_avg_us > queue_avg_us + compute_avg_us) ?
                                  (cumm_avg_us - queue_avg_us - compute_avg_us) :
                                  0;

      msg << "  Avg request latency: " << cumm_avg_us << " usec"
          << "\n"
          << "  (overhead " << overhead << " usec + "
          << "queue " << queue_avg_us << " usec + "
          << "compute input " << compute_input_avg_us << " usec + "
          << "compute infer " << compute_infer_avg_us << " usec + "
          << "compute output " << compute_output_avg_us << " usec)" << std::endl;
    }

    TLOG() << msg.str();
  }

  TritonClient::ServerSideStats TritonClient::summarizeServerStats(
    const inference::ModelStatistics& start_status,
    const inference::ModelStatistics& end_status) const
  {
    TritonClient::ServerSideStats server_stats;

    server_stats.inference_count_ = end_status.inference_count() - start_status.inference_count();
    server_stats.execution_count_ = end_status.execution_count() - start_status.execution_count();
    server_stats.success_count_ = end_status.inference_stats().success().count() -
                                  start_status.inference_stats().success().count();
    server_stats.cumm_time_ns_ =
      end_status.inference_stats().success().ns() - start_status.inference_stats().success().ns();
    server_stats.queue_time_ns_ =
      end_status.inference_stats().queue().ns() - start_status.inference_stats().queue().ns();
    server_stats.compute_input_time_ns_ = end_status.inference_stats().compute_input().ns() -
                                          start_status.inference_stats().compute_input().ns();
    server_stats.compute_infer_time_ns_ = end_status.inference_stats().compute_infer().ns() -
                                          start_status.inference_stats().compute_infer().ns();
    server_stats.compute_output_time_ns_ = end_status.inference_stats().compute_output().ns() -
                                           start_status.inference_stats().compute_output().ns();

    return server_stats;
  }

  inference::ModelStatistics TritonClient::getServerSideStatus() const
  {
    if (verbose_) {
      inference::ModelStatisticsResponse resp;
      bool success = warn_if_error(
        client_->ModelInferenceStatistics(&resp, options_.model_name_, options_.model_version_),
        "getServerSideStatus(): unable to get model statistics");
      if (success) return *(resp.model_stats().begin());
    }
    return inference::ModelStatistics{};
  }

}
