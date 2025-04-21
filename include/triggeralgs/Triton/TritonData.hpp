#ifndef TRIGGERALGS_INCLUDE_TRITON_TRITONDATA_HPP
#define TRIGGERALGS_INCLUDE_TRITON_TRITONDATA_HPP

#include "triggeralgs/Triton/Span.hpp"
#include "triggeralgs/Triton/triton_utils.hpp"
#include "triggeralgs/Triton/TritonIssues.hpp"

#include <algorithm>
#include <any>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include "grpc_client.h"
#include "triton/common/model_config.h"

namespace tc = triton::client;

ERS_DECLARE_ISSUE(
  triggeralgs,
  TritonDataMismatch,
  "TritonData batch size mismatch: expected " << expected << ", got " << got,
  ((size_t)expected)
  ((size_t)got)
)

namespace triggeralgs {
  class TritonClient;

  template <typename DT>
  using TritonInput = std::vector<std::vector<DT>>;
  template <typename DT>
  using TritonOutput = std::vector<triton_span::Span<const DT*>>;

  // Store all the info needed for triton input and output
  template <typename IO>
  class TritonData {
  public:
    using Result = tc::InferResult;
    using TensorMetadata = inference::ModelMetadataResponse_TensorMetadata;
    using ShapeType = std::vector<int64_t>;
    using ShapeView = triton_span::Span<ShapeType::const_iterator>;

    // Constructor
    TritonData(const std::string& name, const TensorMetadata& model_info, bool no_batch);

    // Some members can be modified
    bool set_shape(const ShapeType& new_shape) { return set_shape(new_shape, true); }
    bool set_shape(unsigned loc, int64_t val) { return set_shape(loc, val, true); }

    // I/O accessors
    template <typename DT>
    void to_server(std::shared_ptr<TritonInput<DT>> ptr)
    {
      const auto& data_in = *ptr;

      // Check batch size
      if (data_in.size() != batch_size_) {
        throw triggeralgs::TritonDataMismatch(ERS_HERE, batch_size_, data_in.size());
      }

      //shape must be specified for variable dims or if batch size changes
      data_->SetShape(full_shape_);

      if (byte_size_ != sizeof(DT)) {
        throw triggeralgs::TritonDataByteSizeError(ERS_HERE);
      }

      for (unsigned i0 = 0; i0 < batch_size_; ++i0) {
        const DT* arr = data_in[i0].data();
        triton_utils::fail_if_error(
          data_->AppendRaw(reinterpret_cast<const uint8_t*>(arr), data_in[i0].size() * byte_size_),
          name_ + " input(): unable to set data for batch entry " + std::to_string(i0));
      }

      // Keep input data in scope
      holder_ = std::move(ptr);
    }

    template <typename DT>
    TritonOutput<DT> from_server() const;

    //const accessors
    const ShapeView& get_shape() const { return shape_; }
    int64_t get_byte_size() const { return byte_size_; }
    const std::string& get_dname() const { return dname_; }
    unsigned get_batch_size() const { return batch_size_; }

    //utilities
    bool variable_dims() const { return variable_dims_; }
    int64_t sizeDims() const { return product_dims_; }
    //default to dims if shape isn't filled
    int64_t sizeShape() const { return variable_dims_ ? dim_product(shape_) : sizeDims(); }

  private:
    friend class TritonClient;

    //private accessors only used by client
    bool set_shape(const ShapeType& newShape, bool canThrow);
    bool set_shape(unsigned loc, int64_t val, bool canThrow);
    void set_batch_size(unsigned bsize);
    void reset();
    void set_result(std::shared_ptr<Result> result) { result_ = result; }
    IO* data() { return data_.get(); }

    //helpers
    bool any_negatives(const ShapeView& vec) const
    {
      return std::any_of(vec.begin(), vec.end(), [](int64_t i) { return i < 0; });
    }
    int64_t dim_product(const ShapeView& vec) const
    {
      return std::accumulate(vec.begin(), vec.end(), 1, std::multiplies<int64_t>());
    }
    void create_object(IO** ioptr) const;

    //members
    std::string name_;
    std::shared_ptr<IO> data_;
    const ShapeType dims_;
    bool no_batch_;
    unsigned batch_size_;
    ShapeType full_shape_;
    ShapeView shape_;
    bool variable_dims_;
    int64_t product_dims_;
    std::string dname_;
    inference::DataType dtype_;
    int64_t byte_size_;
    std::any holder_;
    std::shared_ptr<Result> result_;
  };

  using TritonInputData = TritonData<tc::InferInput>;
  using TritonInputMap = std::unordered_map<std::string, TritonInputData>;
  using TritonOutputData = TritonData<tc::InferRequestedOutput>;
  using TritonOutputMap = std::unordered_map<std::string, TritonOutputData>;

  template <>
  void TritonInputData::reset();
  template <>
  void TritonOutputData::reset();
  template <>
  void TritonInputData::create_object(tc::InferInput** ioptr) const;
  template <>
  void TritonOutputData::create_object(tc::InferRequestedOutput** ioptr) const;

  //explicit template instantiation declarations
  extern template class TritonData<tc::InferInput>;
  extern template class TritonData<tc::InferRequestedOutput>;

}
#endif // TRIGGERALGS_TRITON_TRITON_DATA_HPP