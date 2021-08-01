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
  
  //This boolean value is switched to true when a we've collected all the tps in our time window
  activate_algorithm = false;


  //This chunk of code  collects tps into tp_list every 4500 time ticks
  //The first conditional is only for the first input
  if (time_window == 0) {  
    time_window = input_tp.time_start;
    tp_list.push_back(input_tp);

  //The second conditional passes the 4500 tick tp_list into the algorithm
  } else if ( input_tp.time_start > time_window + 4500) {
    time_window = input_tp.time_start;
    activate_algorithm = true; 

  // The third conditional adds the incoming trigger primitive into the algorithm
  } else {
    tp_list.push_back(input_tp);
  }


  if (activate_algorithm) 
  {

    std::cout << "--------------------------------------------------------------------------------------------------------\n";
    std::cout << "Algorithm Activated at time" << tp_list[0].time_start << "\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n"; 


    //Refresh all variables and arrays from previous run
    refresh();   
   
    //This is for the creation of a trigger activity at the end of the code
    timewindow_start = tp_list[0].time_start;
    timewindow_end = tp_list[tp_list.size()-1].time_start;    

    // Shift start times and peak times to be from 0 to 4500
    int64_t shift = tp_list[0].time_start; 
    for (int i=0; i<tp_list.size(); ++i){
      tp_list[i].time_start = (tp_list[i].time_start - shift); 
      tp_list[i].time_peak = (tp_list[i].time_peak - shift);
    }
    
    //Sort TP list by channel
    std::sort (tp_list.begin(), tp_list.end(), compare_channel);
   
    for (int i=0; i<tp_list.size(); ++i){
      initialvec_adc.push_back(tp_list[i].adc_integral);
    } 

  

    //This is for the creation of a trigger activity later on towards the end of the code
    channel_start = (uint16_t)tp_list[0].channel;
    channel_end = (uint16_t)tp_list[tp_list.size()-1].channel;    



//////////////////////////////////Begining of read out////////////////////////////////////////////////////////////////////////////////////////////

    std::cout << "Time Start:  " << timewindow_start << "\t";
    std::cout << "Channel Start:  " << channel_start << "\n";
    std::cout << "Time End:    " << timewindow_end << "\t";
    std::cout << "Channel End:    " << channel_end << "\n";
    std::cout << "Number of Trigger Primitives: " << tp_list.size() << "\n";

    std::cout << "___channel______time_start___time_over_threshold___adc_ingetral___adc_peak_____index___\n";    
    for(int i=0; i<tp_list.size(); i++){
      if (tp_list[i].time_start == 0){
        std::cout << "   " << tp_list[i].channel << "   \t   " <<  tp_list[i].time_start <<  "   \t\t   " <<  tp_list[i].time_over_threshold << "   \t   " <<  tp_list[i].adc_integral << "   \t   " << tp_list[i].adc_peak << "\t\t" << i << "\n";    
      }
      else {
      std::cout << "   " << tp_list[i].channel << "   \t   " <<  tp_list[i].time_start <<  "   \t   " <<  tp_list[i].time_over_threshold << "   \t   " <<  tp_list[i].adc_integral << "   \t   " << tp_list[i].adc_peak << "\t\t" << i <<  "\n";    
      }
    }
    
    //Finding maxADC
    for (int i=0; i<tp_list.size(); ++i){ 
      
      //changes channel box
      if ((tp_list[i].channel > chnlind_vec[boxchcnt]) or (i==tp_list.size()-1)){
        if(tmpchnl_vec.size()==0){
          while(tp_list[i].channel > chnlind_vec[boxchcnt]){
	    boxchcnt+=1;
	  }
      }
      else{
        for(int time_ind=0; time_ind < timeind_vec.size()-1; time_ind++){
	  sublist.clear();
	  for (int tmpch=0; tmpch < tmpchnl_vec.size(); tmpch++){
	    if (tmpchnl_vec[tmpch].time_start >= timeind_vec[time_ind] and tmpchnl_vec[tmpch].time_start < timeind_vec[time_ind+1]){
	      TriggerPrimitive tp { tmpchnl_vec[tmpch].time_start, 0,  tmpchnl_vec[tmpch].time_over_threshold, tmpchnl_vec[tmpch].channel, tmpchnl_vec[tmpch].adc_integral, tmpchnl_vec[tmpch].adc_peak, 0, 0, 0, 0, 0};
              sublist.push_back(tp);
            }}
          maxadc = 0;
	  if(sublist.size()>0){
	    for (int sl=0; sl<sublist.size(); sl++){                             
	      if (sublist[sl].adc_integral> maxadc) {
	        maxadc =  sublist[sl].adc_integral;
	        maxadcind = sl;                                                  
                if(maxadc > braggE){   
                  TriggerPrimitive max_tp { sublist[maxadcind].time_start, 0,  sublist[maxadcind].time_over_threshold, sublist[maxadcind].channel, sublist[maxadcind].adc_integral, sublist[maxadcind].adc_peak, 0, 0, 0, 0, 0};
 
	          tp_list_maxadc.push_back(max_tp);
                  maxadc = 0;
	  }}}}
        }
        tmpchnl_vec.clear();
        }
      }
 
      if (tp_list[i].channel > chnlind_vec[boxchcnt]) boxchcnt+=1;
      if (tp_list[i].channel <= chnlind_vec[boxchcnt] or i==tp_list.size()-1){
        TriggerPrimitive tp {tp_list[i].time_start, 0,  tp_list[i].time_over_threshold, tp_list[i].channel, tp_list[i].adc_integral, tp_list[i].adc_peak, 0, 0, 0, 0, 0};
        tmpchnl_vec.push_back(tp);
      }
    }  


    std::cout << "\n\n";
    
    if (tp_list_maxadc.size() == 0) { 
    std::cout << "\nWith a Bragg threshold of " << braggE << ", no trigger primitives were selected\n";
    }

    else {
 
      std::cout << "\nWith a Bragg threshold of " << braggE << ", " << tp_list_maxadc.size() << " trigger primitives were selected:\n";
      std::cout << "___channel______time_start___time_over_threshold___adc_ingetral___adc_peak_____index___\n";    
      for (int tpt=0; tpt<tp_list_maxadc.size(); tpt++){
        maxadcindex =  getIndex(initialvec_adc, tp_list_maxadc[tpt].adc_integral);
        if(tp_list_maxadc[tpt].time_start == 0) {         
        std::cout << "   " << tp_list_maxadc[tpt].channel << "   \t   " <<  tp_list_maxadc[tpt].time_start <<  "  \t\t   " <<  tp_list_maxadc[tpt].time_over_threshold << "   \t   " <<  tp_list_maxadc[tpt].adc_integral << "   \t   " << tp_list_maxadc[tpt].adc_peak << "\t\t" << maxadcindex << "\n";    
        }
        else {
        std::cout << "   " << tp_list_maxadc[tpt].channel << "   \t   " <<  tp_list_maxadc[tpt].time_start <<  "   \t   " <<  tp_list_maxadc[tpt].time_over_threshold << "   \t   " <<  tp_list_maxadc[tpt].adc_integral << "   \t   " << tp_list_maxadc[tpt].adc_peak << "\t\t" << maxadcindex << "\n";    
        }      
        maxadcindex_vec.push_back(maxadcindex);
      }
    std::cout << "\n\n";
    }
//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\     ONLY FOCUS ON BELOW    /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\

    stc:cout << "Now we begin calculating slopes created by the contiguous channels for each selected TP!\n\n\n";

    for (int imaxadc=0; imaxadc<tp_list_maxadc.size(); imaxadc++){

      chnl_maxadc = tp_list_maxadc[imaxadc].channel;
      time_max = tp_list_maxadc[imaxadc].time_start + tp_list_maxadc[imaxadc].time_over_threshold;
      time_min = tp_list_maxadc[imaxadc].time_start;

      TriggerPrimitive tp_this {tp_list_maxadc[imaxadc].time_start, 0,  tp_list_maxadc[imaxadc].time_over_threshold, tp_list_maxadc[imaxadc].channel, tp_list_maxadc[imaxadc].adc_integral, tp_list_maxadc[imaxadc].adc_peak, 0, 0, 0, 0, 0};
      tp_list_this.push_back(tp_this);
      tp_list_prev = tp_list_this;
      tp_list_next = tp_list_this;
      tp_list_sf = tp_list_this;
      tp_list_sb = tp_list_this;   



      // Initialize again some of the required variables here after starting a loop over maxADC TP
      frontfound = false;
      hitfound = false;
      braggcnt=0;
      slope = 0;
      horiz_noise_cnt = 0;
      frontslope_top = 0;
      backslope_top = 0;
      frontslope_mid = 0;
      backslope_mid = 0;
      frontslope_bottom = 0;
      backslope_bottom = 0;
      frontangle_top = 0;
      frontangle_mid = 0;
      frontangle_bottom = 0;
      backangle_top=0;
      backangle_mid=0;
      backangle_bottom=0;
      horiz_tb = 0;
      horiz_tt = 0;

      maxadcindex =  getIndex(initialvec_adc, tp_list_maxadc[imaxadc].adc_integral);

      //std::cout << "\nAnalyzing channels FORWARD to channel " << tp_list_maxadc[imaxadc].channel << " with adc integral " << tp_list_maxadc[imaxadc].adc_integral << " and index " << maxadcindex << "...\n"  <<  std::endl;

///////////////////////////////////////////////////////////////////////////Checking Forward Channels///////////////////////////////////////////////////////////////////////////////////////

      for (int icheck=maxadcindex; icheck<tp_list.size(); icheck++) {   
        
        //These are the adjacent channels 
        //std::cout << "\n\t/index:    " << icheck << std::endl;
        //std::cout << "\t|channel:  " << tp_list[icheck].channel << std::endl;
        //std::cout << "\t\\braggcnt: " << braggcnt << std::endl;

        //If we've found a front, we move on to the second part of the code
        if (frontfound == true){
          std::cout << "\n\nFRONT FOUND -- ";
          break;
        }

        //If current icheck loop is in channel two channels away either from channel with max adc or channel next to maxadc channel based on if we find hit in next channel (right next to chnl with max adc) 
        if(tp_list[icheck].channel >= (chnl_maxadc+2)){//I changed 2 to 5 here
          chnl_maxadc = tp_list_next[icheck].channel;
 
          //std::cout << "\tCurrent Channel:    "  <<  tp_list_maxadc[imaxadc].channel << "    Next Channel:    " << tp_list_next[imaxadc].channel << std::endl;
          
          if (hitfound == false) break;

          if (hitfound == true){
            braggcnt+=1;
            //std::cout << "\tSince a HIT was found, so the braggcnt is now:     " << braggcnt << std::endl;  
      	    if (braggcnt==3){
              tp_list_sf = tp_list_next; 
              //std::cout << "\tThe braggcnt is now 3" << std::endl;
            }

            if (braggcnt >= tracklen/2){
              frontfound = true;
              int denf = (tp_list_next[imaxadc].channel - tp_list_sf[imaxadc].channel);
              if (denf!=0){
        	    frontslope_top = float(tp_list_next[imaxadc].time_start + tp_list_next[imaxadc].time_over_threshold - tp_list_sf[imaxadc].time_start - tp_list_sf[imaxadc].time_over_threshold)/ denf; 

                    frontslope_mid = float(tp_list_next[imaxadc].time_start + (tp_list_next[imaxadc].time_over_threshold)/2 -tp_list_sf[imaxadc].time_start - (tp_list_sf[imaxadc].time_over_threshold)/2)/ denf ;

		    frontslope_bottom = float(tp_list_next[imaxadc].time_start - tp_list_sf[imaxadc].time_start)/ denf; 
            }
            }
            tp_list_prev = tp_list_next;
          }
        //MIGHT NEED TO MOVE THIS INTO ANOTHER BLOCK
        hitfound = false;
        this_time_max = 0;
        this_time_min = 0;
        prev_time_max = 0;
        prev_time_min = 0;
        }


        if(   (tp_list[icheck].channel == (chnl_maxadc+1)) or    (tp_list[icheck].channel == (chnl_maxadc+2)) or    (tp_list[icheck].channel == (chnl_maxadc+3)) or    (tp_list[icheck].channel == (chnl_maxadc+4)) ){
          //std::cout << "\n\tChecking if time condition has been satisfied for (This Time, Previous Time) = ("<< this_time_min << ", " << tp_list_prev[icheck].time_start <<")" << std::endl;
          this_time_max = tp_list[icheck].time_start + tp_list[icheck].time_over_threshold;
          this_time_min =  tp_list[icheck].time_start;
          prev_time_max = tp_list_prev[imaxadc].time_start + tp_list_prev[imaxadc].time_over_threshold;
          prev_time_min =tp_list_prev[imaxadc].time_start;	

          if ((this_time_min>=prev_time_min and this_time_min<=prev_time_max) or (this_time_max>=prev_time_min and this_time_max<=prev_time_max) or (prev_time_max<=this_time_max and prev_time_min>=this_time_min) ){
            //std::cout << "\tTime Condition: True"  << std::endl;
            if (horiz_noise_cnt == 0){
              horiz_tb = prev_time_min;
              horiz_tt = prev_time_max;
            }

            if (tp_list[icheck].channel == tp_list_next[icheck].channel) break;
            hitfound = true;
            //std::cout << "\n\t///////////////////\n\tHIT HAS BEEN FOUND in channel " << tp_list[icheck].channel << "\n\t///////////////////" << std::endl;
            tp_list_next[imaxadc].channel = tp_list[icheck].channel;   
            tp_list_next[imaxadc].time_start = tp_list[icheck].time_start;
            tp_list_next[imaxadc].adc_integral = tp_list[icheck].adc_integral;
            tp_list_next[imaxadc].adc_peak = tp_list[icheck].adc_peak;
            tp_list_next[imaxadc].time_over_threshold = tp_list[icheck].time_over_threshold;
 

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
      //BACKWARDS CHANNELS /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ 
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
        std::cout << "BACKWARDS PART OF ALGORITHM ACTIVATED" << std::endl;

        for (int icheckb=maxadcindex; icheckb>=0; icheckb--){
          std::cout << "\tMICHEL TRIGGER PROCESSING" << std::endl;
 

          if(tp_list[icheckb].channel <= (chnl_maxadc-2)){
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

  		float num = float(bky1+bky2-(bky3+bky4));
  	       	int den = (tp_list_next[imaxadc].channel - tp_list_sb[imaxadc].channel);
		if(den!=0){
  	          backslope_top = (bky1+bky2-(bky3+bky4)) / den; 
                  backslope_mid = (bky1+(bky2/2)-(bky3+(bky4/2))) / den; 
  	          backslope_bottom = (bky1-bky3) / den; 
  		  frontangle_top = (atan(slopecm_scale*float(frontslope_top)))*radTodeg;
                  backangle_top = (atan(slopecm_scale*float(backslope_top)))*radTodeg;
                  frontangle_mid = (atan(slopecm_scale*float(frontslope_mid)))*radTodeg;
                  backangle_mid = (atan(slopecm_scale*float(backslope_mid)))*radTodeg;
                  frontangle_bottom = (atan(slopecm_scale*float(frontslope_bottom)))*radTodeg;
                  backangle_bottom = (atan(slopecm_scale*float(backslope_bottom)))*radTodeg;
		  if (abs(frontangle_mid-backangle_mid)>30 or abs(frontangle_top-backangle_top)>30 or abs(frontangle_bottom-backangle_bottom)>30){
                    trigtot += 1;
		    //std::cout <<" Trigger: " << trigtot << std::endl;
                    TriggerPrimitive tp_final {tp_list_maxadc[imaxadc].time_start, 0,  tp_list_maxadc[imaxadc].time_over_threshold, tp_list_maxadc[imaxadc].channel, tp_list_maxadc[imaxadc].adc_integral, tp_list_maxadc[imaxadc].adc_peak, 0, 0, 0, 0, 0};
                    std::cout << "\t\tMICHEL TRIGGER FINALIZED" << std::endl;
                    final_tp_list.push_back(tp_final);  
		    //std::cout <<" Trigger AT (bragg peak found at ): " << tp_list_maxadc[imaxadc].channel << " , " << tp_list_maxadc[imaxadc].time_start << " , "<< tp_list_maxadc[imaxadc].adc_integral << std::endl;
		  }
                  else {
                    break;
                  }
	        }
              } 

    	      tp_list_prev = tp_list_next;

    	    }
          //Might need to move this into another block
          hitfound ==false;
          this_time_max = 0;
          this_time_min = 0;
          prev_time_max = 0;
          prev_time_min = 0;
          }
          if( (tp_list[icheckb].channel = (chnl_maxadc-1)) or  (tp_list[icheckb].channel = (chnl_maxadc-2)) or (tp_list[icheckb].channel = (chnl_maxadc-3)) or (tp_list[icheckb].channel = (chnl_maxadc-4)) ){
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
              hitfound = true;

              tp_list_next[imaxadc].channel = tp_list[icheckb].channel;                      
              tp_list_next[imaxadc].time_start = tp_list[icheckb].time_start;
              tp_list_next[imaxadc].adc_integral = tp_list[icheckb].adc_integral;
              tp_list_next[imaxadc].adc_peak = tp_list[icheckb].adc_peak;
              tp_list_next[imaxadc].time_over_threshold = tp_list[icheckb].time_over_threshold;

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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
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
