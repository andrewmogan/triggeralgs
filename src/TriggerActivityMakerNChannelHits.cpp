/**
 * @file TriggerActivityMakerNChannelHits.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/NChannelHits/TriggerActivityMakerNChannelHits.hpp"
#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerNChannelHits"
#include <vector>
#include <math.h>

using namespace triggeralgs;

void
TriggerActivityMakerNChannelHits::operator()(const TriggerPrimitive& input_tp,
					     std::vector<TriggerActivity>& output_ta)
{
  // Add useful info about recived TPs here for FW and SW TPG guys.
  if (m_print_tp_info){
    TLOG(1) << "TP Start Time: " << input_tp.time_start;
  }


  // FIRST TP =====================================================================
  // The first time operator() is called, reset the window object.
  if (m_current_window.is_empty()) {
    m_current_window.reset(input_tp);
    m_primitive_count++;
    return;
  }

  // If the difference between the current TP's start time and the start of the window
  // is less than the specified window size, add the TP to the window.
  if ((input_tp.time_start - m_current_window.time_start) < m_window_length) {
    m_current_window.add(input_tp);
  }

  // MULTIPLICITY - N UNQIUE CHANNELS EXCEEDED =====================================
  // If the addition of the current TP to the window would make it longer than the
  // specified window length, don't add it but check whether the number of hit channels
  // in the existing window is above the specified threshold. If it is, and we are triggering
  // on channel multiplicity, make a TA and start a fresh window with the current TP.
  else if (m_current_window.n_channels_hit() > m_n_channels_threshold) {

    ta_count++;
    if (ta_count % m_prescale == 0){

      TLOG(1) << "Emitting multiplicity trigger with " << m_current_window.n_channels_hit() <<
                  " unique channels hit.";

      output_ta.push_back(construct_ta());
      m_current_window.reset(input_tp);
    }
  }

  // Otherwise, slide the window along using the current TP.
  else {
    m_current_window.move(input_tp, m_window_length);  
  }

  using namespace std::chrono;
  // If this is the first TP of the run, calculate the initial offset:
  if (m_primitive_count == 0){
    m_initial_offset = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - input_tp.time_start*16*1e-6;
  }

  // Update OpMon Variable(s)
  uint64_t system_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  uint64_t data_time = input_tp.time_start*16*1e-6;                              // Convert 62.5 MHz ticks to ms
  m_data_vs_system_time.store(fabs(system_time - data_time - m_initial_offset)); // Store the difference for OpMon*/
  m_primitive_count++;

  return;
}

void
TriggerActivityMakerNChannelHits::configure(const nlohmann::json& config)
{
  if (config.is_object()) {
    if (config.contains("n_channels_threshold"))
      m_n_channels_threshold = config["n_channels_threshold"];
    if (config.contains("window_length"))
      m_window_length = config["window_length"];
    if (config.contains("print_tp_info"))
      m_print_tp_info = config["print_tp_info"];
    if (config.contains("prescale"))
      m_prescale = config["prescale"]; 
 }
}

TriggerActivity
TriggerActivityMakerNChannelHits::construct_ta() const
{

  TriggerPrimitive latest_tp_in_window = m_current_window.inputs.back();

  TriggerActivity ta;
  ta.time_start = m_current_window.time_start;
  //ta.time_end = latest_tp_in_window.time_start + latest_tp_in_window.time_over_threshold;
  ta.time_end = latest_tp_in_window.time_start;
  ta.time_peak = latest_tp_in_window.time_peak;
  ta.time_activity = latest_tp_in_window.time_peak;
  ta.channel_start = latest_tp_in_window.channel;
  ta.channel_end = latest_tp_in_window.channel;
  ta.channel_peak = latest_tp_in_window.channel;
  ta.detid = latest_tp_in_window.detid;
  ta.type = TriggerActivity::Type::kTPC;
  ta.algorithm = TriggerActivity::Algorithm::kNChannelHits;
  ta.inputs = m_current_window.inputs;

  return ta;
}

// =====================================================================================
// Functions below this line are for debugging purposes.
// =====================================================================================
void
TriggerActivityMakerNChannelHits::add_window_to_record(TPWindow window)
{
  m_window_record.push_back(window);
  return;
}

// Function to dump the details of the TA window currently on record
void
TriggerActivityMakerNChannelHits::dump_window_record()
{
  std::ofstream outfile;
  outfile.open("window_record_tam.csv", std::ios_base::app);

  for (auto window : m_window_record) {
    outfile << window.time_start << ",";
    outfile << window.inputs.back().time_start << ",";
    outfile << window.inputs.back().time_start - window.time_start << ",";
    outfile << window.n_channels_hit() << ",";       // Number of unique channels with hits
    outfile << window.inputs.size() << ",";          // Number of TPs in TPWindow
    outfile << window.inputs.back().channel << ",";  // Last TP Channel ID
    outfile << window.inputs.front().channel << ","; // First TP Channel ID
  }

  outfile.close();
  m_window_record.clear();

  return;
}

// Function to add current TP details to a text file for testing and debugging.
void
TriggerActivityMakerNChannelHits::dump_tp(TriggerPrimitive const& input_tp)
{
  std::ofstream outfile;
  outfile.open("coldbox_tps.txt", std::ios_base::app);

  // Output relevant TP information to file
  outfile << input_tp.time_start << " ";          
  outfile << input_tp.time_over_threshold << " "; // 50MHz ticks
  outfile << input_tp.time_peak << " ";           
  outfile << input_tp.channel << " ";             // Offline channel ID
  outfile << input_tp.detid << " ";               // Det ID - Identifies detector element
  outfile << input_tp.type << std::endl;        
  outfile.close();

  return;
}
