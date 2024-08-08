/**
 * @file TAMakerPrescaleAlgorithm.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Prescale/TAMakerPrescaleAlgorithm.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TAMakerPrescaleAlgorithm"

#include <vector>

using namespace triggeralgs;

void
TAMakerPrescaleAlgorithm::process(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta)
{
    std::vector<TriggerPrimitive> tp_list;
    tp_list.push_back(input_tp);

    TriggerActivity ta;
    ta.time_start = input_tp.time_start;
    ta.time_end = input_tp.time_start + input_tp.time_over_threshold;
    ta.time_peak = input_tp.time_peak;
    ta.time_activity = 0;
    ta.channel_start = input_tp.channel;
    ta.channel_end = input_tp.channel;
    ta.channel_peak = input_tp.channel;
    ta.adc_integral = input_tp.adc_integral;
    ta.adc_peak = input_tp.adc_peak;
    ta.detid = input_tp.detid;
    ta.type = TriggerActivity::Type::kTPC;
    ta.algorithm = TriggerActivity::Algorithm::kPrescale;

    ta.inputs = tp_list;

    using namespace std::chrono;

    // Update OpMon Variable(s)
    uint64_t system_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    uint64_t data_time = ta.time_start*16e-6;             // Convert 62.5 MHz ticks to ms    
    m_data_vs_system_time.store(data_time - system_time); // Store the difference for OpMon

    output_ta.push_back(ta);
}

void
TAMakerPrescaleAlgorithm::configure(const nlohmann::json& config)
{
  TriggerActivityMaker::configure(config);
}

// Register algo in TA Factory
REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TAMakerPrescaleAlgorithm)
