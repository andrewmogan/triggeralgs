/**
 * @file TriggerActivityMakerMichel.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERACTIVITYMAKERMICHEL_HPP_
#define TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERACTIVITYMAKERMICHEL_HPP_

#include "triggeralgs/TriggerActivityMaker.hpp"
#include <limits>
#include <vector>

namespace triggeralgs {
class TriggerActivityMakerMichel : public TriggerActivityMaker
{
public:
  TriggerActivityMakerMichel() {

    //Clear before filling
    timeind_vec.clear();
    chnlind_vec.clear();
    
    //Time slices to divide the collection plane channels
    for(int timeind= 0; timeind <= 4500; timeind+=boxwidtime){
      timeind_vec.push_back(timeind);
    }
    
    //Channel slices to divide the collection plane channels
    for(int chnlind=ColPlStartChnl; chnlind<(ColPlEndChnl+boxwidch); chnlind+=boxwidch){ 
      chnlind_vec.push_back(chnlind);       
     }

  }
  
  static bool compare_channel(const TriggerPrimitive& ch_a, const TriggerPrimitive& ch_b) 
  { 
    //smallest comes first                                                                                                      
    return ch_a.channel < ch_b.channel ; // and (ch_a.time_start < ch_b.time_start));                                           
  }

  static int getIndex(std::vector<uint32_t> v, uint32_t K){
    auto it = find(v.begin(), v.end(), K);
    if (it != v.end())
    {   
      int index = it - v.begin();
      return index;   
    }
    else {
      return -99;  
    }
  }

  
  void refresh() {
    boxchcnt = 1;
    braggcnt=0;
    trigtot = 0;
    horiz_noise_cnt = 0;
    initialvec_adc.clear();
    tp_list_maxadc.clear();
    tp_list_this.clear();
    tp_list_prev.clear();
    tp_list_next.clear();
    tp_list_sf.clear();
    tp_list_sb.clear();
    final_tp_list.clear();
  } 

  void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta) override;

protected:
  int64_t m_time_tolerance =
    250; /// Maximum tolerated time difference between two primitives to form an activity (in 50 MHz clock ticks)
  int16_t m_channel_tolerance =
    2; /// Maximum tolerated channel number difference between two primitives to form an activity

private:
  

  bool activate_algorithm = false;

  //For the creation of the trigger activity
  int64_t timewindow_start;
  int64_t timewindow_end;
  uint16_t channel_start;
  uint16_t channel_end;

  uint32_t ch_init = 0;
  int maxadcindex;
  int maxadcind;
  uint16_t maxadc =0;
  uint32_t chnlwid = 0;
  int64_t timewid=0;
  int64_t time_window = 0;
  int64_t minimum_time = 0;
  int64_t maximum_time = 0;
  int counting = 0;
  int trigtot;
  int64_t TPvol, TPdensity;
  int time_diff = 30;
  uint16_t braggE = 4500; //  27500 is used in uB based on incoming muon angle vs maxadc                   
  uint32_t chnl_maxadc;
  int64_t time_max, this_time_max, prev_time_max, horiz_tt;
  int64_t temp_t;
  int64_t time_min, this_time_min, prev_time_min, horiz_tb;
  float slopecm_scale = 0.04/0.3; //time ticks to cm = 1/25, channels to cm = 0.3                            
  bool frontfound = false;
  bool hitfound = false;
  int TPcount=0;
  int braggcnt=0;
  int chcnt=0;
  int horiz_noise_cnt = 0;
  int horiz_tolerance = 8;
  int tracklen=6;//26
  float radTodeg=180/3.14;
  int64_t y2,y1,y3,y4;
  uint32_t x2,x1,x3,x4;
  float bky1,bky2,bky3,bky4, bkpy1,bkpy2,bkpy3,bkpy4;
  float frontangle_top, frontangle_mid, frontangle_bottom, backangle_top, backangle_mid, backangle_bottom;
  float slope, frontslope_top, frontslope_mid, frontslope_bottom, backslope_top, backslope_mid, backslope_bottom;
  int ColPlStartChnl = 0; // from DUNE 0                                                                                    
  int ColPlEndChnl = 2560; //from DUNE 2560 for uBooNE 479                                                                                      
  int boxchcnt = 1;
  uint32_t prev_chnl, next_chnl, sf_chnl;
  int64_t prev_tstart, next_tstart, sf_tstart;
  int32_t prev_tot, next_tot, sf_tot;
  int contiguous_tolerance = 16;
  
  
  std::vector<TriggerPrimitive> tp_for_next_time_window;


  int64_t boxwidtime=1150;
  std::vector<int64_t> timeind_vec;
  int  boxwidch=96; //We might want to change this based on how many channels we have                                                                                                    
  std::vector<uint32_t> chnlind_vec;

  std::vector<TriggerPrimitive> tp_list;
  std::vector<TriggerPrimitive> tp_list_time_shifted; 
  std::vector<TriggerPrimitive> tp_only;
  std::vector<TriggerPrimitive> tp_list_maxadc;
  std::vector<TriggerPrimitive> tp_list_this;
  std::vector<TriggerPrimitive> tp_list_prev;
  std::vector<TriggerPrimitive> tp_list_next;
  std::vector<TriggerPrimitive> tp_list_sf;
  std::vector<TriggerPrimitive> tp_list_sb;
  std::vector<TriggerPrimitive> tmpchnl_vec;
  std::vector<TriggerPrimitive> sublist;
  std::vector<TriggerPrimitive> final_tp_list;
  std::vector<int>  maxadcindex_vec;
  std::vector<uint32_t> initialvec_adc;    //Maybe change this back to original type?
  std::vector<TriggerPrimitive> test;

  int64_t  time_start;
  int32_t  time_over_threshold;
  int64_t  time_peak;
  uint32_t channel ;
  uint16_t adc_integral ;
  uint16_t adc_integralN ;
  uint16_t adc_peak;
  uint32_t detid;
  uint32_t type;

};
} // namespace triggeralgs

#endif // TRIGGERALGS_SRC_TRIGGERALGS_MICHEL_TRIGGERACTIVITYMAKERMICHEL_HPP_
