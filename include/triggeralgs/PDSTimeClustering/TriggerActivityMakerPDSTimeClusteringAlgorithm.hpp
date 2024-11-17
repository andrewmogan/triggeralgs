/**
 * @file TriggerActivityMakerPDSTimeClusteringAlgorithm.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_TIMECLUSTERING_TRIGGERACTIVITYMAKERPDS_HPP_
#define TRIGGERALGS_TIMECLUSTERING_TRIGGERACTIVITYMAKERPDS_HPP_

#include "triggeralgs/TriggerActivityFactoryPDS.hpp"

#include <vector>

namespace triggeralgs {
class TriggerActivityMakerPDSTimeClusteringAlgorithm : public TriggerActivityMakerPDS
{

public:
  void process(const TriggerPrimitivePDS& input_tp, std::vector<TriggerActivityPDS>& output_ta);
  
  void configure(const nlohmann::json &config);
  TriggerActivityPDS::Type m_type = TriggerActivityPDS::Type::kPDS;
  TriggerActivityPDS::Algorithm m_algorithm = TriggerActivityPDS::Algorithm::kPDSTimeClustering;

private:  
  class TPBuffer {
    public:
      bool is_empty() const{
        return tp_list.empty();
      };
      void add(TriggerPrimitivePDS const &input_tp){
        // Add the input TP's contribution to the total ADC and add it to
        // the TP list.
        adc_integral += input_tp.adc_integral;
        time_end=input_tp.time_start+input_tp.time_over_threshold;
        if(adc_peak<input_tp.adc_peak)
        {
          adc_peak=input_tp.adc_peak;
          time_peak=input_tp.time_peak;
        }
        tp_list.push_back(input_tp);
      };
      void clear(){
        tp_list.clear();
      };
      void reset(TriggerPrimitivePDS const &input_tp){
        // Empty the TP list.
        tp_list.clear();
        // Set the start time of the window to be the start time of the 
        // input_tp.
        adc_peak = input_tp.adc_peak;
        time_start = input_tp.time_start;
        time_end = input_tp.time_start+input_tp.time_over_threshold;
        time_peak = input_tp.time_peak;
        adc_integral = input_tp.adc_integral;
        tp_list.push_back(input_tp);
      };
      friend std::ostream& operator<<(std::ostream& os, const TPBuffer& window){
        if(window.is_empty()) os << "Buffer is empty!\n";
        else{
          os << "Window start: " << window.time_start << ", end: " << window.tp_list.back().time_start;
          os << ". Total of: " << window.adc_integral << " ADC counts with " << window.tp_list.size() << " TPs.\n"; 
        }
        return os;
      };

      timestamp_t time_start;
      timestamp_t time_end;
      uint32_t adc_integral;
      uint16_t adc_peak;
      timestamp_t time_peak;
      std::vector<TriggerPrimitivePDS> tp_list;
  };

  TriggerActivityPDS construct_ta() const;

  TPBuffer m_current_tpbuffer;
  uint64_t m_primitive_count = 0;

  // Configurable parameters.
  uint32_t m_adc_threshold = 1200000;
  timestamp_t m_ticks_limit = 50;
  int counter=0;
};
} // namespace triggeralgs

#endif // TRIGGERALGS_TIMECLUSTERING_TRIGGERACTIVITYMAKERPDS_HPP_
