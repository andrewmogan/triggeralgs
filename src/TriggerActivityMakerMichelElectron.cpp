/**
 * @file TriggerActivityMakerMichelElectron.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/MichelElectron/TriggerActivityMakerMichelElectron.hpp"
#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerMichelElectron"
#include <vector>

using namespace triggeralgs;

void
TriggerActivityMakerMichelElectron::operator()(const TriggerPrimitive& input_tp,
                                               std::vector<TriggerActivity>& output_ta)
{
  // The first time operator() is called, reset the window object.
  // dump_tp(input_tp); // For debugging
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

  // Adjacency Threshold Exceeded ============================================================================
  // We've filled the window, and first attempt to identify a track using the adjacency checker from the HMA.
  // When this condition is met, we proceed to check the running mean of the ADC of the TPs of that 'track'.
  else if (check_adjacency() > m_adjacency_threshold && m_trigger_on_adjacency) { 

    auto adjacency = check_adjacency();
    if (adjacency > m_max_adjacency) {
      TLOG(TLVL_DEBUG) << "New max adjacency: previous was " << m_max_adjacency << ", new " << adjacency;
      m_max_adjacency = adjacency;
    }

    if (adjacency > m_adjacency_threshold) {

     // Debugging things    
     // TLOG(1) << "Emitting adjacency TA with adjacency " << adjacency;
     add_window_to_record(m_current_window);
     dump_window_record(); 
    
     // Function to dump start and end channels of tracks leading to TA. Also does the running mean ADC checking
     check_running_adc();

     output_ta.push_back(construct_ta());
     m_current_window.reset(input_tp);
    }
  }

  // Otherwise, slide the window along using the current TP.
  else {
    m_current_window.move(input_tp, m_window_length);
  }

  m_primitive_count++;

  return;
}

void
TriggerActivityMakerMichelElectron::configure(const nlohmann::json& config)
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
TriggerActivityMakerMichelElectron::construct_ta() const
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
  ta.algorithm = TriggerActivity::Algorithm::kMichelElectron;
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
                       TriggerActivity::Algorithm::kMichelElectron,
                       0,
                       m_current_window.inputs};*/
  return ta;
}

/*void TriggerActivityMakerMichelElectron::check_running_adc()
{
  // Idea here is to loop over the TPs in the track (we have a track since adjacency has been met at this point)
  // and make a running average of their ADC integral. Add these running averages to a list, and search that list
  // for a spike indicating a bragg peak later on
  
  std::vector<int> mean_adc_list;
  
  // create a channel ordered list of tps, their channel and adc values
    struct hit {
    int chan;
    uint32_t adc;
  };
  std::vector<hit> hitList;
  for (auto tp : m_current_window.inputs) {
     hitList.push_back({tp.channel, tp.adc_integral});
  }
  std::sort(hitList.begin(), hitList.end(), [](hit a, hit b) {
                return a.chan < b.chan; });

  // Loop through this list, checking adcs 1 behind, 1 inront and averaging
  float adc_mean = 1;
  uint16_t max = 0;
  unsigned int current_adc = 0;
  unsigned int next_adc = 0;
  unsigned int next = 0;
  unsigned int prev = 0 ;
  unsigned int prev_adc = 0;

  for(unsigned int i = 0; i < hitList.size(); ++i){
    next = (i+1) % hitList.size();
    prev = (i-1) % hitList.size();
    current_adc = hitList.at(i).adc;
    next_adc = hitList.at(next).adc;   
    prev_adc = hitList.at(prev).adc;

    mean_adc_list.push_back((current_adc + prev_adc + next_adc)/3);

  }

  std::ofstream outfile;
  outfile.open("running_adc_means.csv", std::ios_base::app);

  for (auto a : mean_adc_list){
    outfile << a << ",";
  } 
  outfile << "0" << std::endl; // end the line with fake value 0, move to next window
  outfile.close();

  TLOG(1) << "List of running average ADCs created. Length of the list is: " << mean_adc_list.size();
  return;
}*/


uint16_t
TriggerActivityMakerMichelElectron::check_adjacency() const
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
  // ===============================================================================================================
  // Adjcancency Tolerance = Number of wires we're willing to skip before resetting the adjacency count.
  // This accounts for things like dead channels / missed TPs. Modifiied from HMA, will skip up to 3 consecutive wires.
  for (unsigned int i = 0; i < chanList.size(); ++i) {

    next = (i + 1) % chanList.size(); // Loops back when outside of channel list range
    channel = chanList.at(i);
    next_channel = chanList.at(next); // Next channel with a hit

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
    // So we're allowing up to 3 missed hits now, but each missed hit tolls up!
    else if ((next_channel == channel + 4) && (tol_count < m_adj_tolerance)) {
      ++adj;
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
TriggerActivityMakerMichelElectron::add_window_to_record(Window window)
{
  m_window_record.push_back(window);
  return;
}


// Function to dump the details of the TA window currently on record
void
TriggerActivityMakerMichelElectron::dump_window_record()
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

// Function to dump the start and end channels of the track triggered on. Also checks the running
// mean of ADC for a spike, i.e. searching for a Bragg peak for decaying muon.
void
TriggerActivityMakerMichelElectron::check_running_adc()
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
    uint32_t adc; 
  };
  
  std::vector<hit> chanList;  // Full list of hits to loop through
  std::vector<hit> trackHits; // List of all hits that contribute to the trakc/adjacency 
  std::vector<hit> finalHits; // Final list of hits that make up track
  for (auto tp : m_current_window.inputs) {
     chanList.push_back({tp.channel, tp.time_start, tp.adc_integral});
  }
  // sort chanList by the channel number, since we want to find the largest consecutive chain of them
  std::sort(chanList.begin(), chanList.end(), [](hit a, hit b) {
		return a.chan < b.chan;	});

  // ADAJACENCY METHOD 1 ===========================================================================================
  // ====================================================================================================
  // Adjcancency Tolerance = Number of times willing to skip a single missed wire before
  // resetting the adjacency count. This accounts for things like dead channels / missed TPs.
  s = chanList.at(0).chan; 
  st = chanList.at(0).startTime;

  for (unsigned int i = 0; i < chanList.size(); ++i) {

    next = (i + 1) % chanList.size(); // Loops back when outside of channel list range
    channel = chanList.at(i).chan;
    next_channel = chanList.at(next).chan; // Next channel with a hit
    
    // Initialise the vector
    if (trackHits.size() == 0){
	trackHits.push_back(chanList.at(i));
    }

    // End of vector condition
    if (next_channel == 0) {
      next_channel = channel - 1;
    }

    // Skip same channel hits
    if (next_channel == channel) {
      // But if the same channel hit is within few 100 ticks, accept it's ADC contribution to the track
      int diff = chanList.at(next).startTime - chanList.at(i).startTime;
      if(std::abs(diff) < 200){
      trackHits.push_back(chanList.at(next)); // Still want this hits ADC contribution!
      // Idea here is that two hits generated on the same wire, very close in time are
      // likely coming from the same particle/track activity.
      }
      continue;
    }

    // If next hit is on next channel, increment the adjacency count
    else if (next_channel == channel + 1) {
      ++adj;
      e = next_channel;
      et = chanList.at(next).startTime;
      trackHits.push_back(chanList.at(next));
     }

    // If next channel is not on the next hit, but the 'second next',
    // increase adjacency but also tally up with the tolerance counter.
    else if ((next_channel == channel + 2) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
      e = next_channel;
      et = chanList.at(next).startTime;
      trackHits.push_back(chanList.at(next));
    }

    else if ((next_channel == channel + 3) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
      ++tol_count;
      e = next_channel;
      et = chanList.at(next).startTime;
      trackHits.push_back(chanList.at(next));
    }

    else if ((next_channel == channel + 4) && (tol_count < m_adj_tolerance)) {
      ++adj;
      ++tol_count;
      ++tol_count;
      ++tol_count;
      e = next_channel;
      et = chanList.at(next).startTime;
      trackHits.push_back(chanList.at(next));
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
        finalHits = trackHits; // final hits are the ones we use to print out the info to a file
       }
      // 
      adj = 1;
      tol_count = 0;
      s = next_channel; // Set the start channel to the next TP channel in the list, not the count is broken
      st = chanList.at(next).startTime;
      trackHits.clear(); // This stops it happening on the next loop for the size 0 condition OR just wipe it here?
    } 
 }  // End of loop through the hit channels


  // Output the start and end channel of this window to the debugging file
  std::ofstream outfile;
  outfile.open("adjacnecy_start_end_tps.csv", std::ios_base::app);
  outfile << fs << "," << fe  << "," << fst << "," << fet << std::endl;
  outfile.close();

  // Write out the Chan, Time & ADC values of TPs that contributed to the track
  std::ofstream outfile2;
  outfile2.open("track_tps.csv", std::ios_base::app);
  for(auto h : finalHits){
    outfile2 << h.chan << "," << h.startTime << "," <<  h.adc << std::endl;;
  }
  outfile2.close();
  // TLOG(1) << "Written all track TPs ADCs contribution to file, number of hits in track (including same chans): " << finalHits.size();

  return;
}


// Function to add current TP details to a text file for testing and debugging.
void
TriggerActivityMakerMichelElectron::dump_tp(TriggerPrimitive const& input_tp)
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
TriggerActivityMakerMichelElectron::check_tot() const
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
TriggerActivityMakerMichelElectron::flush(timestamp_t, std::vector<TriggerActivity>& output_ta)
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
