/**
 * @file TriggerActivityMakerHorizontalMuon.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/HorizontalMuon/TriggerActivityMakerHorizontalMuon.hpp"
#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerHorizontalMuon"
#include <vector>

using namespace triggeralgs;

void
TriggerActivityMakerHorizontalMuon::operator()(const TriggerPrimitive& input_tp,
                                               std::vector<TriggerActivity>& output_ta)
{
  if(input_tp.channel > 2623 && input_tp.channel < 3200){

  // 0) FIRST TP =============================================================================================
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

  // 1) ADC THRESHOLD EXCEEDED ===============================================================================
  // If the addition of the current TP to the window would make it longer specified
  // window length, don't add it but check whether the ADC integral if the existing
  // window is above the configured threshold. If it is, and we are triggering on ADC,
  // make a TA and start a fresh window with the current TP.
  else if (m_current_window.adc_integral > m_adc_threshold && m_trigger_on_adc) {
    output_ta.push_back(construct_ta());
    m_current_window.reset(input_tp);
  }

  // 2) N UNQIUE CHANNELS EXCEEDED ===========================================================================
  // If the addition of the current TP to the window would make it longer than the
  // specified window length, don't add it but check whether the number of hit channels
  // in the existing window is above the specified threshold. If it is, and we are
  // triggering on channel multiplicity, make a TA and start a fresh window with the current TP.
  else if (m_current_window.n_channels_hit() > m_n_channels_threshold && m_trigger_on_n_channels) {

    // add_window_to_record(m_current_window); // For debugging
    //dump_window_record(); // For debugging
    // TLOG(1) << "Emitting multiplicity trigger."; // For debugging

    output_ta.push_back(construct_ta());
    m_current_window.reset(input_tp);
  }

  // 3) ADJACENCY THRESHOLD EXCEEDED =========================================================================
  // If the addition of the current TP to the window would make it longer than the
  // specified window length, don't add it but check whether the adjacency of the
  // current window exceeds the configured threshold. If it does, and we are triggering
  // on adjacency, then create a TA and reset the window with the new/current TP.
  else if (check_adjacency() > m_adjacency_threshold &&  m_trigger_on_adjacency) {
    
    //auto adjacency = check_adjacency();    

   // TLOG(1) << "Emitting adjacency TA with adjacency " << adjacency;
    // add_window_to_record(m_current_window); // For debugging
    // dump_window_record(); // For debugging
    // dump_adjacency_channels(); // Function to dump start and end channels of tracks leading to TA

    output_ta.push_back(construct_ta());
    m_current_window.reset(input_tp);
  }

  // 4) Otherwise, slide the window along using the current TP.
  else {
    m_current_window.move(input_tp, m_window_length);
  }

  m_primitive_count++;
  } // End of collection filter

  return;
}

void
TriggerActivityMakerHorizontalMuon::configure(const nlohmann::json& config)
{
  // FIX ME: Use some schema here. Also can't work out how to pass booleans.
  if (config.is_object()) {
    if (config.contains("trigger_on_adc"))
      m_trigger_on_adc = config["trigger_on_adc"];
    if (config.contains("trigger_on_n_channels"))
      m_trigger_on_n_channels = config["trigger_on_n_channels"];
    if (config.contains("adc_threshold"))
      m_adc_threshold = config["adc_threshold"];
    if (config.contains("n_channels_threshold"))
      m_n_channels_threshold = config["n_channels_threshold"];
    if (config.contains("window_length"))
      m_window_length = config["window_length"];
    if (config.contains("trigger_on_adjacency"))
      m_trigger_on_adjacency = config["trigger_on_adjacency"];
    if (config.contains("adj_tolerance"))
      m_adj_tolerance = config["adj_tolerance"];
    if (config.contains("adjacency_threshold"))
      m_adjacency_threshold = config["adjacency_threshold"];
  }

  // m_conf = config.get<dunedaq::triggeralgs::triggeractivitymakerhorizontalmuon::ConfParams>();
}

TriggerActivity
TriggerActivityMakerHorizontalMuon::construct_ta() const
{

  TriggerPrimitive latest_tp_in_window = m_current_window.inputs.back();

  TriggerActivity ta;
  ta.time_start = m_current_window.time_start;
  ta.time_end = latest_tp_in_window.time_start + latest_tp_in_window.time_over_threshold;
  ta.time_peak = latest_tp_in_window.time_peak;
  ta.time_activity = latest_tp_in_window.time_peak;
  ta.channel_start = latest_tp_in_window.channel;
  ta.channel_end = latest_tp_in_window.channel;
  ta.channel_peak = latest_tp_in_window.channel;
  ta.adc_integral = m_current_window.adc_integral;
  ta.adc_peak = latest_tp_in_window.adc_peak;
  ta.detid = latest_tp_in_window.detid;
  ta.type = TriggerActivity::Type::kTPC;
  ta.algorithm = TriggerActivity::Algorithm::kHorizontalMuon;
  ta.inputs = m_current_window.inputs;

  /*  TriggerActivity ta{m_current_window.time_start,
                       latest_tp_in_window.time_start+latest_tp_in_window.time_over_threshold,
                       latest_tp_in_window.time_peak,
                       latest_tp_in_window.time_peak,
                       latest_tp_in_window.channel,
                       latest_tp_in_window.channel,
                       latest_tp_in_window.channel,
                       m_current_window.adc_integral,
                       latest_tp_in_window.adc_peak,
                       latest_tp_in_window.detid,
                       TriggerActivity::Type::kTPC,
                       TriggerActivity::Algorithm::kHorizontalMuon,
                       0,
                       m_current_window.inputs};*/
  return ta;
}

uint16_t
TriggerActivityMakerHorizontalMuon::check_adjacency() const
{
  // This function returns the adjacency value for the current window, where adjacency
  // is defined as the maximum number of consecutive wires containing hits. It accepts
  // a configurable tolerance paramter, which allows up to adj_tolerance single hit misses
  // on adjacent wires before restarting the adjacency count.

  uint16_t adj = 1;
  uint16_t max = 0; // Maximum adjacency of window, which this function returns
  unsigned int channel = 0;
  unsigned int next_channel = 0;
  unsigned int next = 0;
  unsigned int tol_count = 0;

  // Start and end channels of track - for debugging function
  uint16_t s = 1;
  uint16_t e = 1;

  // Generate a channelID ordered list of hit channels for this window
  std::vector<int> chanList;
  for (auto tp : m_current_window.inputs) {
    chanList.push_back(tp.channel);
  }
  std::sort(chanList.begin(), chanList.end());

  // ADAJACENCY METHOD 1 ===========================================================================================
  // ====================================================================================================
  // Adjcancency Tolerance = Number of times willing to skip a single missed wire before
  // resetting the adjacency count. This accounts for things like dead channels / missed TPs.
  for (unsigned int i = 0; i < chanList.size(); ++i) {

    next = (i + 1) % chanList.size(); // Loops back when outside of channel list range
    channel = chanList.at(i);
    next_channel = chanList.at(next); // Next channel with a hit
    int gap = 5; // Max number of wires to skip before ending count

    // End of vector condition
    if (next_channel == 0) {
      next_channel = channel - 1;
    }

    // Skip same channel hits
    if (next_channel == channel) {
      continue;
    }

    // If next hit is on next channel, increment the adjacency count (and update endChannel:debugging)
    else if (next_channel == channel + 1){
      ++adj;
    }

    // If next channel is not on the next hit, but the 'second next',
    // increase adjacency but also tally up with the tolerance counter.
    else if ((next_channel == channel + 2) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
    }

    else if ((next_channel == channel + 3) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
      ++tol_count;
    }

    else if ((next_channel == channel + 4) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
      ++tol_count;
      ++tol_count;
    }

    else if ((next_channel == channel + 5) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
      ++tol_count;
      ++tol_count;
      ++tol_count;
    }

    // If next hit isn't within our reach, end adj count and check for a new max
    else {
      if (adj > max) {
        max = adj;
      }
      adj = 1;
      tol_count = 0;
    }

 }

  // ADJACENCY METHOD 2 ==========================================================================================
  // ===================================================================================================
  // Adjacency Tolerance = Number of consecutive missed wires you're willing to skip
  // before resetting the adjacency count.
  /* for (unsigned int i=0; i < chanList.size(); ++i){

       next = (i+1)%chanList.size();
       channel = chanList.at(i);
       next_channel = chanList.at(next);

       if (next_channel == 0){
          next_channel=channel-1;
       }

       if (next_channel == channel){ continue; }

       else if ((next_channel - channel) < m_adj_tolerance) { ++adj; }

       else {
         if (adj > max){
               max = adj;
       }
       adj = 1;
       tol_count = 0;
       }
    }*/

  return max;
}

// Functions below this line are for debugging purposes.
void
TriggerActivityMakerHorizontalMuon::add_window_to_record(Window window)
{
  m_window_record.push_back(window);
  return;
}


// Function to dump the details of the TA window currently on record
void
TriggerActivityMakerHorizontalMuon::dump_window_record()
{
  // FIX ME: Need to index this outfile in the name by detid or something similar.
  std::ofstream outfile;
  outfile.open("window_record_tam.csv", std::ios_base::app);

  for (auto window : m_window_record) {
    outfile << window.time_start << ",";
    outfile << window.inputs.back().time_start << ",";
    outfile << window.inputs.back().time_start - window.time_start << ","; // window_length - from TP start times
    outfile << window.adc_integral << ",";
    outfile << window.n_channels_hit() << ",";       // Number of unique channels with hits
    outfile << window.inputs.size() << ",";          // Number of TPs in Window
    outfile << window.inputs.back().channel << ",";  // Last TP Channel ID
    outfile << window.inputs.front().channel << ","; // First TP Channel ID
    outfile << check_adjacency() << ",";             // New adjacency value for the window
    outfile << check_tot() << std::endl;             // Summed window TOT
  }

  outfile.close();

  m_window_record.clear();

  return;
}

// Function to dump the start and end channels of the track triggered on
void
TriggerActivityMakerHorizontalMuon::dump_adjacency_channels()
{
 
 // Copy of the adjacency method 1
  int adj = 1;
  int max = 0; // Maximum adjacency of window, which this function returns
  unsigned int channel = 0;
  unsigned int next_channel = 0;
  unsigned int next = 0;
  unsigned int tol_count = 0; 

  // Start and end channels of track - for debugging function
  uint16_t s = 1;
  uint16_t e = 1;
  uint16_t fs = 1;
  uint16_t fe = 1;
  long unsigned int st = 0;
  long unsigned int et = 0;
  long unsigned int fst = 0;
  long unsigned int fet = 0;

  // Generate a channelID ordered list of hit channels for this window
  struct hit {
    int chan;
    long unsigned int startTime; 
  };  
  std::vector<hit> chanList; 
  for (auto tp : m_current_window.inputs) {
     chanList.push_back({tp.channel, tp.time_start});
  }
  std::sort(chanList.begin(), chanList.end(), [](hit a, hit b) {
		return a.chan < b.chan;	});

  // ADAJACENCY METHOD 1 ===========================================================================================
  // ====================================================================================================
  // Adjcancency Tolerance = Number of times willing to skip a single missed wire before
  // resetting the adjacency count. This accounts for things like dead channels / missed TPs.
  s = chanList.at(0).chan; // Initialise start channel 
  st = chanList.at(0).startTime;

  for (unsigned int i = 0; i < chanList.size(); ++i) {

    next = (i + 1) % chanList.size(); // Loops back when outside of channel list range
    channel = chanList.at(i).chan;
    next_channel = chanList.at(next).chan; // Next channel with a hit

    // End of vector condition
    if (next_channel == 0) {
      next_channel = channel - 1;
    }

    // Skip same channel hits
    if (next_channel == channel) {
      continue;
    }

    // If next hit is on next channel, increment the adjacency count (and update endChannel:debugging)
    else if (next_channel == channel + 1) {
      ++adj;
      e = next_channel;
      et = chanList.at(next).startTime;
    }

    // If next channel is not on the next hit, but the 'second next',
    // increase adjacency but also tally up with the tolerance counter.
    else if ((next_channel == channel + 2) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
      e = next_channel;
      et = chanList.at(next).startTime;
    }

    // If next hit isn't within our reach, end adj count and check for a new max
    // Also finalise the start and end channels of the adjacency of this window
    else {
      if (adj > max) {
        max = adj;
        fs = s;
	fe = e;
        fst = st;
        fet = et; 
      }
      // 
      adj = 1;
      tol_count = 0;
      s = next_channel; // Set the start channel to the next TP channel in the list, not the count is broken
      st = chanList.at(next).startTime;
    } 
 }  // End of loop through the hit channels


  // Output the start and end channel of this window to the debugging file
  std::ofstream outfile;
  outfile.open("adjacnecy_start_end_tps.csv", std::ios_base::app);

  outfile << fs << ",";
  outfile << fe   << ",";
  outfile << fst << ",";
  outfile << fet   << std::endl;

  outfile.close();

  return;
}


// Function to add current TP details to a text file for testing and debugging.
void
TriggerActivityMakerHorizontalMuon::dump_tp(TriggerPrimitive const& input_tp)
{
  std::ofstream outfile;
  outfile.open("coldbox_tps.txt", std::ios_base::app);

  // Output relevant TP information to file
  outfile << input_tp.time_start << " ";          // Start time of TP
  outfile << input_tp.time_over_threshold << " "; // in multiples of 25
  outfile << input_tp.time_peak << " ";           //
  outfile << input_tp.channel << " ";             // Offline channel ID
  outfile << input_tp.adc_integral << " ";        // ADC Sum
  outfile << input_tp.adc_peak << " ";            // ADC Peak Value
  outfile << input_tp.detid << " ";               // Det ID - Identifies detector element, APA or PDS part etc...
  outfile << input_tp.type << std::endl;          // This should now write out TPs in the same 'coldBox' way.
  outfile.close();

  return;
}

int
TriggerActivityMakerHorizontalMuon::check_tot() const
{
  // Here, we just want to sum up all the tot values for each TP within window, and return this tot of the window.
  int window_tot = 0; // The window TOT, which this function returns

  for (auto tp : m_current_window.inputs) {
    window_tot += tp.time_over_threshold;
  }

  return window_tot;
}

/*
void
TriggerActivityMakerHorizontalMuon::flush(timestamp_t, std::vector<TriggerActivity>& output_ta)
{
  // Check the status of the current window, construct TA if conditions are met. Regardless
  // of whether the conditions are met, reset the window.
  if(m_current_window.adc_integral > m_adc_threshold && m_trigger_on_adc){
  //else if(m_current_window.adc_integral > m_conf.adc_threshold && m_conf.trigger_on_adc){
    //TLOG_DEBUG(TRACE_NAME) << "ADC integral in window is greater than specified threshold.";
    output_ta.push_back(construct_ta());
  }
  else if(m_current_window.n_channels_hit() > m_n_channels_threshold && m_trigger_on_n_channels){
  //else if(m_current_window.n_channels_hit() > m_conf.n_channels_threshold && m_conf.trigger_on_n_channels){
    //TLOG_DEBUG(TRACE_NAME) << "Number of channels hit in the window is greater than specified threshold.";
    output_ta.push_back(construct_ta());
  }

  //TLOG_DEBUG(TRACE_NAME) << "Clearing the current window, on the arrival of the next input_tp, the window will be
reset."; m_current_window.clear();

  return;
}*/
