/**
 * @file TriggerCandidateMakerNChannelHits.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/NChannelHits/TriggerCandidateMakerNChannelHits.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerCandidateMakerNChannelHits"

#include <vector>
#include <math.h>
using namespace triggeralgs;

void
TriggerCandidateMakerNChannelHits::operator()(const TriggerActivity& activity,
                                                std::vector<TriggerCandidate>& output_tc)
{

  std::vector<TriggerActivity::TriggerActivityData> ta_list = { static_cast<TriggerActivity::TriggerActivityData>(
    activity) };

  // Find the offset for the very first data vs system time measure:
  if (m_activity_count == 0) {
    using namespace std::chrono;
    m_initial_offset = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - activity.time_start*16*1e-6; 
  }

  // The first time operator is called, reset window object.
  if (m_current_window.is_empty()) {
    m_current_window.reset(activity);
    m_activity_count++;

    TriggerCandidate tc = construct_tc();
    output_tc.push_back(tc);

    using namespace std::chrono;

    // Update OpMon Variable(s)
    uint64_t system_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    uint64_t data_time = m_current_window.time_start*16*1e-6;                      // Convert 62.5 MHz ticks to ms    
    m_data_vs_system_time.store(fabs(system_time - data_time - m_initial_offset)); // Store the difference for OpMon
    
    m_current_window.clear();
    return;
  }

  // If the difference between the current TA's start time and the start of the window
  // is less than the specified window size, add the TA to the window.
  else if ((activity.time_start - m_current_window.time_start) < m_window_length) {
    // TLOG_DEBUG(TRACE_NAME) << "Window not yet complete, adding the activity to the window.";
    m_current_window.add(activity);
  }

  // If the addition of the current TA to the window would make it longer
  // than the specified window length, don't add it but check whether the number of hit channels in
  // the existing window is above the specified threshold. If it is, and we are triggering on channels,
  // make a TC and start a fresh window with the current TA.
  else if (m_current_window.n_channels_hit() > m_n_channels_threshold) {
    tc_number++;
    output_tc.push_back(construct_tc());
    m_current_window.reset(activity);
  }

  // If it is not, move the window along.
  else {
    // TLOG_DEBUG(TRACE_NAME) << "TAWindow is at required length but specified threshold not met, shifting window along.";
    m_current_window.move(activity, m_window_length);
  }

  m_activity_count++;
  return;
}


void
TriggerCandidateMakerNChannelHits::configure(const nlohmann::json& config)
{
  if (config.is_object()) {
    if (config.contains("window_length"))
      m_window_length = config["window_length"];
    if (config.contains("readout_window_ticks_before"))
      m_readout_window_ticks_before = config["readout_window_ticks_before"];
    if (config.contains("readout_window_ticks_after"))
      m_readout_window_ticks_after = config["readout_window_ticks_after"];
  }

  return;
}

TriggerCandidate
TriggerCandidateMakerNChannelHits::construct_tc() const
{
  TriggerActivity latest_ta_in_window = m_current_window.inputs.back();

  TriggerCandidate tc;
  tc.time_start = m_current_window.time_start - m_readout_window_ticks_before;
  tc.time_end = m_current_window.time_start + m_readout_window_ticks_after;
  //tc.time_end = latest_ta_in_window.inputs.back().time_start + latest_ta_in_window.inputs.back().time_over_threshold;
  tc.time_candidate = m_current_window.time_start;
  tc.detid = latest_ta_in_window.detid;
  tc.type = TriggerCandidate::Type::kNChannelHits;
  tc.algorithm = TriggerCandidate::Algorithm::kNChannelHits;

  // Take the list of triggeralgs::TriggerActivity in the current
  // window and convert them (implicitly) to detdataformats'
  // TriggerActivityData, which is the base class of TriggerActivity
  for (auto& ta : m_current_window.inputs) {
    tc.inputs.push_back(ta);
  }

  return tc;
}

// Functions below this line are for debugging purposes.
void
TriggerCandidateMakerNChannelHits::add_window_to_record(TAWindow window)
{
  m_window_record.push_back(window);
  return;
}

void
TriggerCandidateMakerNChannelHits::dump_window_record()
{
  // FIX ME: Need to index this outfile in the name by detid or something similar.
  std::ofstream outfile;
  outfile.open("window_record_tcm.csv", std::ios_base::app);

  for (auto window : m_window_record) {
    outfile << window.time_start << ",";
    outfile << window.inputs.back().time_start << ",";
    outfile << window.inputs.back().time_start - window.time_start << ",";
    outfile << window.n_channels_hit() << ",";
    outfile << window.inputs.size() << std::endl;
  }

  outfile.close();

  m_window_record.clear();

  return;
}
