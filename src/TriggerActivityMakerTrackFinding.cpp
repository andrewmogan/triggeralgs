/**
 * @file TriggerActivityMakerTrackFinding.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/TrackFinding/TriggerActivityMakerTrackFinding.hpp"
#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerTrackFindingPlugin"
#include <vector>
#include <math.h>

using namespace triggeralgs;

void
TriggerActivityMakerTrackFinding::operator()(const TriggerPrimitive& input_tp,
					     std::vector<TriggerActivity>& output_ta)
{

  // Add useful info about recived TPs here for FW and SW TPG guys.
  if (m_print_tp_info){
    TLOG(1) << " ########## m_current_window is reset ##########\n"
	    << " TP Start Time: " << input_tp.time_start << ", TP ADC Sum: " << input_tp.adc_integral
	    << ", TP TOT: " << input_tp.time_over_threshold << ", TP ADC Peak: " << input_tp.adc_peak
	    << ", TP Offline Channel ID: " << input_tp.channel << "\n";
  }
  
  // 0) FIRST TP =====================================================================
  // The first time operator() is called, reset the window object.
  if (m_current_window.is_empty()) {
    m_current_window.reset(input_tp);
    m_primitive_count++;
    return;
  }
  
  // If the difference between the current TP's start time and the start of the window
  // is less than the specified window size, add the TP to the window.
  bool adj_pass = 0; // sets to true when adjacency logic is satisfied
  bool window_filling = 0; // sets to true when window is ready to test the adjacency logic
  if ((input_tp.time_start - m_current_window.time_start) < m_window_length) {
    m_current_window.add(input_tp);
    window_filling = 1;
     TLOG(1) << "m_current_window.time_start " << m_current_window.time_start << "\n";
  }
  
  else {
    TPWindow win_adj_max;

    bool ta_found = 1;
    while (ta_found) {
      
      // make m_current_window_tmp a copy of m_current_window and clear m_current_window
      TPWindow m_current_window_tmp;
      for (auto tp : m_current_window.inputs) m_current_window_tmp.add(tp);
      m_current_window.clear();
      
      // make m_current_window a new window of non-overlapping tps (of m_current_window_tmp and win_adj_max)                                                                                 
      for (auto tp : m_current_window_tmp.inputs) {
	bool new_tp = 1;
	for (auto tp_sel : win_adj_max.inputs) {
	  if (tp.channel == tp_sel.channel) new_tp = 0;
	}
	if (new_tp) m_current_window.add(tp);
      }
      
      // check adjacency -> win_adj_max now contains only those tps that make the track
      win_adj_max = check_adjacency();
      if (win_adj_max.inputs.size() > 0) {
    	
	adj_pass = 1;
	ta_found = 1;
	ta_count++;
	if (ta_count % m_prescale == 0){
	  
	  output_ta.push_back(construct_ta(win_adj_max));
	}
      }
      else ta_found = 0;
    }
    if (adj_pass) m_current_window.reset(input_tp);
  }
  
  // if adjacency logic is not true, slide the window along using the current TP.
  if (!window_filling && !adj_pass) {
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
TriggerActivityMakerTrackFinding::configure(const nlohmann::json& config)
{
  if (config.is_object()) {
    if (config.contains("window_length"))
      m_window_length = config["window_length"];
    if (config.contains("adj_tolerance"))
      m_adj_tolerance = config["adj_tolerance"];
    if (config.contains("adjacency_threshold"))
      m_adjacency_threshold = config["adjacency_threshold"];
    if (config.contains("print_tp_info"))
      m_print_tp_info = config["print_tp_info"];
    if (config.contains("prescale"))
      m_prescale = config["prescale"]; 
  }
}

TriggerActivity
TriggerActivityMakerTrackFinding::construct_ta(TPWindow win_adj_max) const
{
  
  TriggerActivity ta;
  
  TriggerPrimitive last_tp = win_adj_max.inputs.back();
  
  ta.time_start = last_tp.time_start;
  ta.time_end = last_tp.time_start+last_tp.time_over_threshold;
  ta.time_peak = last_tp.time_peak;
  ta.time_activity = last_tp.time_peak;
  ta.channel_start = last_tp.channel;
  ta.channel_end = last_tp.channel;
  ta.channel_peak = last_tp.channel;
  ta.adc_integral = win_adj_max.adc_integral;
  ta.adc_peak = last_tp.adc_integral;
  ta.detid = last_tp.detid;
  ta.type = TriggerActivity::Type::kTPC;
  ta.algorithm = TriggerActivity::Algorithm::kTrackFinding;
  ta.inputs = win_adj_max.inputs;

  for( const auto& tp : ta.inputs ) {
    ta.time_start = std::min(ta.time_start, tp.time_start);
    ta.time_end = std::max(ta.time_end, tp.time_start+tp.time_over_threshold);
    ta.channel_start = std::min(ta.channel_start, tp.channel);
    ta.channel_end = std::max(ta.channel_end, tp.channel);
    if (tp.adc_peak > ta.adc_peak ) {
      ta.time_peak = tp.time_peak;
      ta.adc_peak = tp.adc_peak;
      ta.channel_peak = tp.channel;
    }
  }

  return ta;
}

//std::vector<TriggerPrimitive>
TPWindow
TriggerActivityMakerTrackFinding::check_adjacency() {
  // This function deals with tp window (m_current_window), select adjacent tps (with a channel gap from 0 to 4; sum of all gaps < m_adj_tolerance),
  // checks if track length > m_adjacency_threshold: return the tp window (win_adj_max, which is subset of the input tp window)

  unsigned int channel = 0;      // Current channel ID
  unsigned int next_channel = 0; // Next channel ID
  unsigned int next = 0;         // The next position in the hit channels vector
  unsigned int tol_count = 0;    // Tolerance count, should not pass adj_tolerance

  // Generate a channelID ordered list of hit channels for this window; second element of pair is tps
  std::vector< std::pair <int, TriggerPrimitive> > chanTPList;
  for (auto tp : m_current_window.inputs) {
    chanTPList.push_back( std::make_pair(tp.channel, tp) );
  }
  std::sort(chanTPList.begin(), chanTPList.end(), [](const std::pair<int,TriggerPrimitive> &a, const std::pair<int,TriggerPrimitive> &b){return (a.first < b.first);});
  
  // ADAJACENCY LOGIC ====================================================================
  // =====================================================================================
  // Adjcancency Tolerance = Number of times prepared to skip missed hits before resetting 
  // the adjacency count (win_adj). This accounts for things like dead channels / missed TPs. The 
  // maximum gap is 4 which comes from tuning on December 2021 coldbox data, and June 2022 
  // coldbox runs.

  TPWindow win_adj;              // add first tp, and then if tps are on next channels (check code below to understand the definition)
  TPWindow win_adj_max;          // if track length > m_adjacency_threshold, set win_adj_max = win_adj; return win_adj_max;
  
  for (int i = 0; i < chanTPList.size(); ++i) {

    win_adj_max.clear();

    next = (i + 1) % chanTPList.size();       // Loops back when outside of channel list range
    channel = chanTPList.at(i).first;
    next_channel = chanTPList.at(next).first; // Next channel with a hit

    // End of vector condition.
    if (next == 0) { next_channel = channel - 1; }
    
    // Skip same channel hits.
    if (next_channel == channel) continue;

    // If win_adj size == zero, add current tp
    if (win_adj.inputs.size() == 0) win_adj.add(chanTPList[i].second);

    // If next hit is on next channel, increment the adjacency count
    if (next_channel - channel == 1) {
      win_adj.add(chanTPList[next].second);
    }
    
    // Allow a max gap of 4 channels (e.g., 45 and 50; 46, 47, 48, 49 are missing); increment the adjacency count
    // Sum of gaps should be < adj_tolerance (e.g., if toleance is 30, the max total gap can vary from 0 to 29+4 = 33)
    else if (next_channel - channel > 0 && next_channel - channel <= 5 &&
	     tol_count < m_adj_tolerance) {
      win_adj.add(chanTPList[next].second);
      for (int i = 0; i < next_channel-channel-1; ++i) ++tol_count;
    }

    // if track length > m_adjacency_threshold, set win_adj_max = win_adj;
    else if (win_adj.inputs.size() > m_adjacency_threshold) {
      win_adj_max = win_adj;
      break;
    }
    
    // If track length < m_adjacency_threshold, reset variables for next iteration.
    else {
      tol_count = 0;
      win_adj.clear();
    }
  }
  
  return win_adj_max;
}

// =====================================================================================
// Functions below this line are for debugging purposes.
// =====================================================================================
void
TriggerActivityMakerTrackFinding::add_window_to_record(TPWindow window)
{
  m_window_record.push_back(window);
  return;
}

// Function to dump the details of the TA window currently on record
void
TriggerActivityMakerTrackFinding::dump_window_record()
{
  std::ofstream outfile;
  outfile.open("window_record_tam.csv", std::ios_base::app);

  for (auto window : m_window_record) {
    outfile << window.time_start << ",";
    outfile << window.inputs.back().time_start << ",";
    outfile << window.inputs.back().time_start - window.time_start << ",";
    outfile << window.adc_integral << ",";
    outfile << window.n_channels_hit() << ",";       // Number of unique channels with hits
    outfile << window.inputs.size() << ",";          // Number of TPs in TPWindow
    outfile << window.inputs.back().channel << ",";  // Last TP Channel ID
    outfile << window.inputs.front().channel << ","; // First TP Channel ID
    outfile << check_tot() << std::endl;             // Summed window TOT
  }

  outfile.close();
  m_window_record.clear();

  return;
}

// Function to add current TP details to a text file for testing and debugging.
void
TriggerActivityMakerTrackFinding::dump_tp(TriggerPrimitive const& input_tp)
{
  std::ofstream outfile;
  outfile.open("coldbox_tps.txt", std::ios_base::app);

  // Output relevant TP information to file
  outfile << input_tp.time_start << " ";          
  outfile << input_tp.time_over_threshold << " "; // 50MHz ticks
  outfile << input_tp.time_peak << " ";           
  outfile << input_tp.channel << " ";             // Offline channel ID
  outfile << input_tp.adc_integral << " ";        
  outfile << input_tp.adc_peak << " ";            
  outfile << input_tp.detid << " ";               // Det ID - Identifies detector element
  outfile << input_tp.type << std::endl;        
  outfile.close();

  return;
}

int
TriggerActivityMakerTrackFinding::check_tot() const
{
  // Here, we just want to sum up all the tot values for each TP within window,
  // and return this tot of the window.
  int window_tot = 0; 
  for (auto tp : m_current_window.inputs) {
    window_tot += tp.time_over_threshold;
  }

  return window_tot;
}

// Register algo in TA Factory
REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerTrackFinding)
