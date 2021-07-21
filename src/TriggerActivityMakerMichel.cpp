/**
 * @file TriggerActivityMakerMichel.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "triggeralgs/Michel/TriggerActivityMakerMichel.hpp"
#include <chrono>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <limits>
#include <cstddef>
#include <stdint.h>
#include <algorithm>

using namespace triggeralgs;
using namespace std;
//WHEN DEBUGGING, REMEMBER THAT ERRORS CAN RESULT FROM ARRAYS THAT HAVE NOT BEEN CLEARED
void
TriggerActivityMakerMichel::operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta)
{
  
  //This boolean value is switched to true when a we've
  //collected all the tps in our time window
  activate_algorithm = false;

  //This if statement clears the previous tp_list and 
  //makes a new one every 4500 ticks
  if (time_window == 0) {
    time_window = input_tp.time_start;
    tp_list.push_back(input_tp);
  } else if ( input_tp.time_start > time_window + 4500) {
    time_window = input_tp.time_start;
    activate_algorithm = true; 
    std::cout << "This is the element of the next array:    " << input_tp.channel << "    " << input_tp.time_start << "\n"; 
  } else {
    tp_list.push_back(input_tp);
  }


  if (activate_algorithm) 
  {
    
    //This is for the creation of a trigger activity later on towards the end of the code
    timewindow_start = tp_list[0].time_start;
    timewindow_end = tp_list[tp_list.size()-1].time_start;    


    // Shift start times to be from 0 to 4500
    int64_t shift = tp_list[0].time_start; 
    for (int i=0; i<tp_list.size(); ++i){
      tp_list[i].time_start = (tp_list[i].time_start - shift); 
    }

 
    //need to refresh vectors from previous run
    timeind_vec.clear();
    chnlind_vec.clear();
    tp_list_maxadc.clear();

    //Time slices to divide the collection plane channels
    for(int timeind= 0; timeind <= 4500; timeind+=boxwidtime){
      timeind_vec.push_back(timeind);
    }
        
    //Channel slices to divide the collection plane channels
    for(int chnlind=ColPlStartChnl; chnlind<(ColPlEndChnl+boxwidch); chnlind+=boxwidch){ 
      chnlind_vec.push_back(chnlind);       
      std::cout << chnlind << "\n";                                                                                         
     }

    //Sort TP list by channel
    std::sort (tp_list.begin(), tp_list.end(), compare_channel);
    

    //This is for the creation of a trigger activity later on towards the end of the code
    channel_start = (uint16_t)tp_list[0].channel;
    channel_end = (uint16_t)tp_list[tp_list.size()-1].channel;    


    //Algorithm always stops at the second to last trigger primitive,
    //so we need to pad it with one tempty rigger primitive
    TriggerPrimitive empty_tp;
    tp_list.push_back(empty_tp);
    
    //Reset to original value
    boxchcnt = 1;
    

    //Finding maxADC
    for (int i=0; i<tp_list.size(); ++i){ 
      
      //changes channel box
      if (tp_list[i].channel > chnlind_vec[boxchcnt] or i==tp_list.size()-1){
        while(tp_list[i].channel > chnlind_vec[boxchcnt]){
	      boxchcnt+=1;
	}
      
      if (tmpchnl_vec.size() != 0 or i==tp_list.size()){      
        for(int time_ind=0; time_ind < timeind_vec.size()-1; time_ind++){
	  sublist.clear();
	  for (int tmpch=0; tmpch < tmpchnl_vec.size(); tmpch++){
	    if (tmpchnl_vec[tmpch].time_start >= timeind_vec[time_ind] and tmpchnl_vec[tmpch].time_start < timeind_vec[time_ind+1]){
	      TriggerPrimitive tp { tmpchnl_vec[tmpch].time_start, 0,  tmpchnl_vec[tmpch].time_over_threshold, tmpchnl_vec[tmpch].channel, tmpchnl_vec[tmpch].adc_integral, tmpchnl_vec[tmpch].adc_peak, 0, 0, 0, 0, 0};
              sublist.push_back(tp);
            }}
          std::cout << "tp_list    " << tp_list.size() << "    sublist    " << sublist.size() << "\n";
	  if(sublist.size()>0 or i==tp_list.size()){
	    for (int sl=0; sl<sublist.size(); sl++){                             
	      if (sublist[sl].adc_integral> maxadc) {
	        maxadc =  sublist[sl].adc_integral;
	        maxadcind = sl;                                                  
                if(maxadc > braggE){   
                  TriggerPrimitive max_tp { sublist[maxadcind].time_start, 0,  sublist[maxadcind].time_over_threshold, sublist[maxadcind].channel, sublist[maxadcind].adc_integral, sublist[maxadcind].adc_peak, 0, 0, 0, 0, 0};
 
	          tp_list_maxadc.push_back(max_tp);
	  }}}}
	  maxadc = 0;
        }
        tmpchnl_vec.clear();
        }
      }
 
      if (tp_list[i].channel <= chnlind_vec[boxchcnt] or i==tp_list.size()-1){
        TriggerPrimitive tp {tp_list[i].time_start, 0,  tp_list[i].time_over_threshold, tp_list[i].channel, tp_list[i].adc_integral, tp_list[i].adc_peak, 0, 0, 0, 0, 0};
        tmpchnl_vec.push_back(tp);
      }
    }  
    
   
    std::cout << "Number of maxADC tps:    " << tp_list_maxadc.size() << "\n";


    for (int tpt=0; tpt<tp_list_maxadc.size(); tpt++){
      maxadcindex =  getIndex(initialvec_adc, tp_list_maxadc[tpt].adc_integral);
      maxadcindex_vec.push_back(maxadcindex);
    }
  
 
    for (int imaxadc=0; imaxadc<tp_list_maxadc.size(); imaxadc++){
      chnl_maxadc = tp_list_maxadc[imaxadc].channel;
      time_max = tp_list_maxadc[imaxadc].time_start + tp_list_maxadc[imaxadc].time_over_threshold;
      time_min = tp_list_maxadc[imaxadc].time_start;

      tp_list_this.push_back({tp_list_maxadc[imaxadc].channel, tp_list_maxadc[imaxadc].time_start, tp_list_maxadc[imaxadc].adc_integral, tp_list_maxadc[imaxadc].adc_peak, tp_list_maxadc[imaxadc].time_over_threshold});

      tp_list_prev = tp_list_this;
      tp_list_next = tp_list_this;
      tp_list_sf = tp_list_this;
      tp_list_sb = tp_list_this;  

      frontfound = false;
      hitfound = false;

      maxadcindex =  getIndex(initialvec_adc, tp_list_maxadc[imaxadc].adc_integral);

      for (int icheck=maxadcindex; icheck<tp_list.size(); icheck++){     
        if (frontfound == true) break;

        if(tp_list[icheck].channel >= (chnl_maxadc+2)){
          chnl_maxadc = tp_list_next[icheck].channel;

          if (hitfound == false) break;

          if (hitfound == true){
            braggcnt+=1;

      	    if (braggcnt==3){
              tp_list_sf = tp_list_next; 
            }

            if (braggcnt >= tracklen/2){
              frontfound = true;
        	    frontslope_top = (tp_list_next[icheck].time_start + tp_list_next[icheck].time_over_threshold - tp_list_sf[icheck].time_start - tp_list_sf[icheck].time_over_threshold)/(tp_list_next[icheck].channel - tp_list_sf[icheck].channel);
              frontslope_mid = (tp_list_next[icheck].time_start + (tp_list_next[imaxadc].time_over_threshold)/2 -tp_list_sf[icheck].time_start - (tp_list_sf[icheck].time_over_threshold)/2)/(tp_list_next[icheck].channel - tp_list_sf[icheck].channel);
        	    frontslope_bottom = (tp_list_next[icheck].time_start - tp_list_sf[icheck].time_start)/(tp_list_next[icheck].channel - tp_list_sf[icheck].channel);
        	  }

            tp_list_prev = tp_list_next;
         	}
        }

        hitfound = false;
        this_time_max = 0;
        this_time_min = 0;
        prev_time_max = 0;
        prev_time_min = 0;
       
        if(tp_list[icheck].channel == (chnl_maxadc+1)){
        	this_time_max = tp_list[icheck].time_start + tp_list[icheck].time_over_threshold;
          this_time_min =  tp_list[icheck].time_start;
          prev_time_max = tp_list_prev[imaxadc].time_start + tp_list_prev[imaxadc].time_over_threshold;
          prev_time_min =tp_list_prev[imaxadc].time_start;	

        	if ((this_time_min>=prev_time_min and this_time_min<=prev_time_max) or (this_time_max>=prev_time_min and this_time_max<=prev_time_max) or (prev_time_max<=this_time_max and prev_time_min>=this_time_min) ){
        	  if (horiz_noise_cnt == 0){
              horiz_tb = prev_time_min;
              horiz_tt = prev_time_max;
        	  }

            if (tp_list[icheck].channel == tp_list_next[icheck].channel) break;

            hitfound == true;
         	  tp_list_next = tp_list;

            if (abs(this_time_min - horiz_tb) <=1 or abs(this_time_max - horiz_tt) <=1){
              horiz_noise_cnt+=1;
              if (horiz_noise_cnt>horiz_tolerance) break;   
            }
            else{
               horiz_noise_cnt = 0;
        	  }

            if (this_time_max > time_max) time_max = this_time_max;
            if (this_time_min < time_min) time_min = this_time_min;

          }
        }
      }
     
      chnl_maxadc = tp_list_maxadc[imaxadc].channel;
      tp_list_prev = tp_list_this;
      tp_list_next = tp_list_this;
      this_time_max =0;
      this_time_min =0;
      prev_time_max = 0;
      prev_time_min =0;
      slope=0;
      hitfound = false;
      
      if (frontfound == true){
        for (int icheckb=maxadcindex; icheckb>=0; --icheckb){
          if(tp_list[icheckb].channel <= (chnl_maxadc+2)){
    	      chnl_maxadc = tp_list_next[imaxadc].channel;
    	      if (hitfound == false) break;
    	      if (hitfound == true) {
    	        braggcnt+=1;
    	        if (braggcnt == tracklen/2+3){
    	          tp_list_sb = tp_list_next;
    	        }
    	        if (braggcnt >= tracklen){
            		bky1=tp_list_next[imaxadc].time_start;
            		bky2=tp_list_next[imaxadc].time_over_threshold;
            		bky3=tp_list_sb[imaxadc].time_start;
            		bky4=tp_list_sb[imaxadc].time_over_threshold;
            		backslope_top = float(bky1+bky2-bky3-bky4)/float(tp_list_next[imaxadc].channel-tp_list_sb[imaxadc].channel);
            		backslope_mid = float(bky1+bky2/2-bky3-bky4/2)/float(tp_list_next[imaxadc].channel-tp_list_sb[imaxadc].channel);
            		backslope_bottom = float(bky1-bky3)/float(tp_list_next[imaxadc].channel-tp_list_sb[imaxadc].channel);
            		frontangle_top = (atan(slopecm_scale*float(frontslope_top)))*radTodeg;
            		backangle_top = (atan(slopecm_scale*float(backslope_top)))*radTodeg;
            		frontangle_mid = (atan(slopecm_scale*float(frontslope_mid)))*radTodeg;
            		backangle_mid = (atan(slopecm_scale*float(backslope_mid)))*radTodeg;
            		frontangle_bottom = (atan(slopecm_scale*float(frontslope_bottom)))*radTodeg;
            		backangle_bottom = (atan(slopecm_scale*float(backslope_bottom)))*radTodeg;
    		        if (abs(frontangle_mid-backangle_mid)>50 and abs(frontangle_top-backangle_top)>50 and abs(frontangle_bottom-backangle_bottom)>50){
    		          trigtot += 1;
    		          final_tp_list.push_back({tp_list_maxadc[imaxadc].channel, tp_list_maxadc[imaxadc].time_start, tp_list_maxadc[imaxadc].adc_integral, tp_list_maxadc[imaxadc].adc_peak, tp_list_maxadc[imaxadc].time_over_threshold});
    		        }
    		        else {
    		          break;
    		        }
    	        }
    		      tp_list_prev = tp_list_next;
    	      }
          }
          hitfound ==false;
          this_time_max = 0;
          this_time_min = 0;
          prev_time_max = 0;
          prev_time_min = 0;
          if(tp_list[icheckb].channel = (chnl_maxadc-1)){
            this_time_max = tp_list[icheckb].time_start + tp_list[icheckb].time_over_threshold;
            this_time_min =  tp_list[icheckb].time_start;
            prev_time_max = tp_list_prev[icheckb].time_start + tp_list_prev[icheckb].time_over_threshold;
            prev_time_min =tp_list_prev[icheckb].time_start;
            if ((this_time_min>=prev_time_min and this_time_min<=prev_time_max) or (this_time_max>=prev_time_min and this_time_max<=prev_time_max) or (prev_time_max<=this_time_max and prev_time_min>=this_time_min) ) {
      	      if (horiz_noise_cnt == 0){
    	      	  horiz_tb = prev_time_min;
    		        horiz_tt = prev_time_max;
    	        }
              if (tp_list[icheckb].channel == tp_list_next[imaxadc].channel) break;
              hitfound == true;
              tp_list_next = tp_list;
              if (abs(this_time_min - horiz_tb) <=1 or abs(this_time_max - horiz_tt) <=1){
                horiz_noise_cnt+=1;
                if (horiz_noise_cnt>horiz_tolerance) break;
              }
              else{
                horiz_noise_cnt = 0;
              }
              if (this_time_max > time_max) time_max = this_time_max;
              if (this_time_min < time_min) time_min = this_time_min;
    	      }
          }
        }
      }
    }
 
    std::cout << "This is the number of primitives collected in run " << time_window << ":    "  << final_tp_list.size() << "\n";
    
    TriggerActivity ta { 
      timewindow_start, 
      timewindow_end, 
      0, //Not sure what to do here
      /*time_activity*/ 0, 
      channel_start, 
      channel_end,
      0, //Not sure what to do for the elements below
      0,
      0, 
      0, 
      0, 
      0, 
      0, 
      final_tp_list //This is the most important part! Candidate Maker will check if it is empty or not
    };

    //Pass to TriggerCandidateMaker
    output_ta.push_back(ta);
 
    //This is for the next run   
    tp_list.clear();
    tp_list.push_back(input_tp);

  }
}
