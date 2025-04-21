#include "TRACE/trace.h"
#include "triggeralgs/Triton/TritonData.hpp"

#include <cstring>
#include <sstream>

namespace ni = triton::common;
namespace tc = triton::client;

namespace triggeralgs {

  //dims: kept constant, represents config.pbtxt parameters of model (converted from google::protobuf::RepeatedField to vector)
  //fullShape: if batching is enabled, first entry is batch size; values can be modified
  //shape: view into fullShape, excluding batch size entry
  template <typename IO>
  TritonData<IO>::TritonData(const std::string& name,
                             const TritonData<IO>::TensorMetadata& model_info,
                             bool no_batch)
    : name_(name)
    , dims_(model_info.shape().begin(), model_info.shape().end())
    , no_batch_(no_batch)
    , batch_size_(0)
    , full_shape_(dims_)
    , shape_(full_shape_.begin() + (no_batch_ ? 0 : 1), full_shape_.end())
    , variable_dims_(any_negatives(shape_))
    , product_dims_(variable_dims_ ? -1 : dim_product(shape_))
    , dname_(model_info.datatype())
    , dtype_(ni::ProtocolStringToDataType(dname_))
    , byte_size_(ni::GetDataTypeByteSize(dtype_))
  {
    //create input or output object
    IO* iotmp;
    create_object(&iotmp);
    data_.reset(iotmp);
  }

  template <>
  void TritonInputData::create_object(tc::InferInput** ioptr) const
  {
    tc::InferInput::Create(ioptr, name_, full_shape_, dname_);
  }

  template <>
  void TritonOutputData::create_object(tc::InferRequestedOutput** ioptr) const
  {
    tc::InferRequestedOutput::Create(ioptr, name_);
  }

  // Setters
  template <typename IO>
  bool TritonData<IO>::set_shape(const TritonData<IO>::ShapeType& new_shape, bool canThrow)
  {
    bool result = true;
    for (unsigned i = 0; i < new_shape.size(); ++i) {
      result &= set_shape(i, new_shape[i], canThrow);
    }
    return result;
  }

  template <typename IO>
  bool TritonData<IO>::set_shape(unsigned loc, int64_t val, bool canThrow)
  {
    std::stringstream msg;
    unsigned full_loc = loc + (no_batch_ ? 0 : 1);

    //check boundary
    if (full_loc >= full_shape_.size()) {
      msg << name_ << " setShape(): dimension " << full_loc << " out of bounds ("
          << full_shape_.size() << ")";
      if (canThrow) {
      // TODO Add exceptions
        //throw cet::exception("TritonDataError") << msg.str();
        TLOG() << "Oops";
        exit(1);
      }
      else {
        //MF_LOG_WARNING("TritonDataWarning") << msg.str();
        return false;
      }
    }

    if (val != full_shape_[full_loc]) {
      if (dims_[full_loc] == -1) {
        full_shape_[full_loc] = val;
        return true;
      }
      else {
        msg << name_ << " set_shape(): attempt to change value of non-variable shape dimension "
            << loc;
        if (canThrow)
          throw TritonDataByteSizeError(ERS_HERE);
        else {
          throw TritonDataByteSizeError(ERS_HERE);
          return false;
        }
      }
    }

    return true;
  }

  template <typename IO>
  void TritonData<IO>::set_batch_size(unsigned bsize)
  {
    batch_size_ = bsize;
    if (!no_batch_) full_shape_[0] = batch_size_;
  }

  //io accessors

  template <>
  template <typename DT>
  TritonOutput<DT> TritonOutputData::from_server() const
  {
    if (!result_) {
      throw triggeralgs::TritonDataByteSizeError(ERS_HERE);
    }

    if (byte_size_ != sizeof(DT)) {
      throw triggeralgs::TritonDataByteSizeError(ERS_HERE);
    }

    uint64_t n_output = sizeShape();
    TritonOutput<DT> dataOut;
    const uint8_t* r0;
    size_t content_byte_size;
    size_t expected_content_byte_size = n_output * byte_size_ * batch_size_;
    triton_utils::fail_if_error(result_->RawData(name_, &r0, &content_byte_size),
                                "output(): unable to get raw");
    if (content_byte_size != expected_content_byte_size) {
      throw triggeralgs::TritonDataByteSizeError(ERS_HERE);
    }

    const DT* r1 = reinterpret_cast<const DT*>(r0);
    dataOut.reserve(batch_size_);
    for (unsigned i0 = 0; i0 < batch_size_; ++i0) {
      auto offset = i0 * n_output;
      dataOut.emplace_back(r1 + offset, r1 + offset + n_output);
    }

    return dataOut;
  }

  template <>
  void TritonInputData::reset()
  {
    data_->Reset();
    holder_.reset();
  }

  template <>
  void TritonOutputData::reset()
  {
    result_.reset();
  }

  //explicit template instantiation declarations
  template class TritonData<tc::InferInput>;
  template class TritonData<tc::InferRequestedOutput>;

  template void TritonInputData::to_server(std::shared_ptr<TritonInput<float>> data_in);
  template void TritonInputData::to_server(std::shared_ptr<TritonInput<int64_t>> data_in);

  template TritonOutput<float> TritonOutputData::from_server() const;
  template TritonOutput<int32_t> TritonOutputData::from_server() const;

}
