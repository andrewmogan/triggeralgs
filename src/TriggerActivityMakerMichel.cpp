#include "triggeralgs/Michel/TriggerActivityMakerMichel.hpp"

#include <chrono>
#include <vector>
#include <array>
#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <limits>
#include <cstddef>
#include <stdint.h>
#include <algorithm>

using namespace std;
using namespace triggeralgs;

void
TriggerActivityMakerMichel::operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta)
{

  //lol
  //
  //
  //
  activate_algorithm = false;


  if (time_window == 0) {  
    time_window = input_tp.time_start;
    tp_list.push_back(input_tp);

  } else if ( input_tp.time_start > time_window + 112500) {
    time_window = input_tp.time_start;
    activate_algorithm = true; 

  } else {
    tp_list.push_back(input_tp);
  }


  if (activate_algorithm) 
  {

    std::cout << "--------------------------------------------------------------------------------------------------------\n";
    std::cout << "Trigger Primitives Collected:    " << tp_list.size() << "\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n"; 
    

    refresh();   
   
    //Shift start times and peak times to be from 0 to 4500 samples
    int64_t shift = tp_list[0].time_start;
    for (int i=0; i<tp_list.size(); ++i){
      tp_list[i].time_start = (tp_list[i].time_start - shift);
      tp_list[i].time_peak = (tp_list[i].time_peak - shift);
      tp_list[i].time_start = tp_list[i].time_start/25;
      tp_list[i].time_peak = tp_list[i].time_peak/25;
      tp_list[i].time_over_threshold = tp_list[i].time_over_threshold/25;
    }   

    std::sort (tp_list.begin(), tp_list.end(), compare_channel);

    for(int i=0; i < tp_list.size();i++){
      if (tp_list[i].channel < 960){
        tp_list_cut.push_back(tp_list[i]);
      }
      else {
        break;
      }
    } 

    //std::cout << "DEBUGGING:    size of tp_list: " << tp_list.size() << "        size of tp_list_cut: " << tp_list_cut.size() << std::endl;
    for (int i=0; i<tp_list_cut.size(); ++i){
      initialvec_adc.push_back(tp_list_cut[i].adc_integral);
    }

    //For displaying the TPs in a table
    std::cout << "___channel______time_start___time_over_threshold___adc_ingetral___adc_peak_____index___\n";
    for(int i=0; i<tp_list_cut.size(); i++){
      if (tp_list_cut[i].time_start == 0){
	std::cout << "   " << tp_list_cut[i].channel << "   \t   " <<  tp_list_cut[i].time_start <<  "   \t\t   " <<  tp_list_cut[i].time_over_threshold << "   \t   " <<  tp_list_cut[i].adc_integral << "   \t   " << tp_list_cut[i].adc_peak << "\t\t" << i << "\n";
      }
      else if (tp_list_cut[i].time_over_threshold < 10){
	std::cout << "   " << tp_list_cut[i].channel << "   \t   " <<  tp_list_cut[i].time_start <<  "   \t   " <<  tp_list_cut[i].time_over_threshold << "   \t\t   " <<  tp_list_cut[i].adc_integral << "   \t   " << tp_list_cut[i].adc_peak << "\t\t" << i << "\n";
      }
      else {
        std::cout << "   " << tp_list_cut[i].channel << "   \t   " <<  tp_list_cut[i].time_start <<  "   \t   " <<  tp_list_cut[i].time_over_threshold << "   \t   " <<  tp_list_cut[i].adc_integral << "   \t   " << tp_list_cut[i].adc_peak << "\t\t" << i <<  "\n";
}}



    for (int i=0; i<tp_list_cut.size(); ++i){
      if ((tp_list_cut[i].channel > chnlind_vec[boxchcnt]) or (i==tp_list_cut.size()-1)){
        if(tmpchnl_vec.size()==0){
          while(tp_list_cut[i].channel > chnlind_vec[boxchcnt]){
            boxchcnt+=1;
          }
        }

        else{
          for(int time_ind=0; time_ind < timeind_vec.size()-1; time_ind++){
            sublist.clear();
      	    for (int tmpch=0; tmpch < tmpchnl_vec.size(); tmpch++){
              if (tmpchnl_vec[tmpch].time_start >= timeind_vec[time_ind] and tmpchnl_vec[tmpch].time_start < timeind_vec[time_ind+1]){

                //TriggerPrimitive tp { tmpchnl_vec[tmpch].time_start}; //test
                TriggerPrimitive tp { tmpchnl_vec[tmpch].time_start, tmpchnl_vec[tmpch].time_peak,  tmpchnl_vec[tmpch].time_over_threshold, tmpchnl_vec[tmpch].channel, tmpchnl_vec[tmpch].adc_integral, tmpchnl_vec[tmpch].adc_peak, tmpchnl_vec[tmpch].detid, tmpchnl_vec[tmpch].type, tmpchnl_vec[tmpch].algorithm, tmpchnl_vec[tmpch].version, tmpchnl_vec[tmpch].flag}; //test
                sublist.push_back(tp);
            }}

            maxadc = 0;
            if(sublist.size()>0){
            for (int sl=0; sl<sublist.size(); sl++){
              if (sublist[sl].adc_integral> maxadc) {
                maxadc =  sublist[sl].adc_integral;
                maxadcind = sl;
            }}

	    if(maxadc > braggE){
              TriggerPrimitive max_tp { sublist[maxadcind].time_start, sublist[maxadcind].time_peak,  sublist[maxadcind].time_over_threshold, sublist[maxadcind].channel, sublist[maxadcind].adc_integral, sublist[maxadcind].adc_peak, sublist[maxadcind].detid, sublist[maxadcind].type, sublist[maxadcind].algorithm, sublist[maxadcind].version, sublist[maxadcind].flag};
  	      tp_list_maxadc.push_back(max_tp);
              maxadc = 0;
      	    }}
          }
        tmpchnl_vec.clear();
        }
      }

      if (tp_list_cut[i].channel > chnlind_vec[boxchcnt]) boxchcnt+=1;

      if (tp_list_cut[i].channel <= chnlind_vec[boxchcnt] or i==tp_list_cut.size()-1){
        TriggerPrimitive tp {tp_list_cut[i].time_start, tp_list_cut[i].time_peak,  tp_list_cut[i].time_over_threshold, tp_list_cut[i].channel, tp_list_cut[i].adc_integral, tp_list_cut[i].adc_peak, tp_list_cut[i].detid, tp_list_cut[i].type,  tp_list_cut[i].algorithm, tp_list_cut[i].version, tp_list_cut[i].flag};
        tmpchnl_vec.push_back(tp);
      }
    }



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


    for (int imaxadc=0; imaxadc<tp_list_maxadc.size(); imaxadc++){
      chnl_maxadc = tp_list_maxadc[imaxadc].channel;
      std::cout << "\n###############  " << chnl_maxadc << "  ###############" << "\n-------------------------------------" << std::endl;
      time_max = tp_list_maxadc[imaxadc].time_start + tp_list_maxadc[imaxadc].time_over_threshold;
      time_min = tp_list_maxadc[imaxadc].time_start;

      TriggerPrimitive tp_this {tp_list_maxadc[imaxadc].time_start, tp_list_maxadc[imaxadc].time_peak, tp_list_maxadc[imaxadc].time_over_threshold, tp_list_maxadc[imaxadc].channel, tp_list_maxadc[imaxadc].adc_integral, tp_list_maxadc[imaxadc].adc_peak, tp_list_maxadc[imaxadc].detid, tp_list_maxadc[imaxadc].type, tp_list_maxadc[imaxadc].algorithm, tp_list_maxadc[imaxadc].version, tp_list_maxadc[imaxadc].flag};
      tp_list_this.push_back(tp_this);
      tp_list_prev = tp_list_this;
      tp_list_next = tp_list_this;
      tp_list_sf = tp_list_this;
      tp_list_sb = tp_list_this;

      initialize();

      maxadcindex =  getIndex(initialvec_adc, tp_list_maxadc[imaxadc].adc_integral);


      for (int icheck=maxadcindex; icheck<tp_list.size(); icheck++) {


          std::cout << "Previous TP";
            std::cout << "\t\t|channel:     " << tp_list_next[imaxadc].channel <<std::endl;
        std::cout << "\t\t\t\t|time_start:  " << tp_list_next[imaxadc].time_start  << "\n" <<std::endl;

        std::cout << "Current TP";
        std::cout << "\t\t|channel:     " << tp_list[icheck].channel << std::endl;
        std::cout << "\t\t\t\t|time_start:  " << tp_list[icheck].time_start<< std::endl;

        if (frontfound == true){
          break;
        }

        if(tp_list[icheck].channel >= (chnl_maxadc+2)){
 
          chnl_maxadc = tp_list_next[imaxadc].channel;

          if (hitfound == false) break;

          if (hitfound == true){
            braggcnt+=1;
            std::cout << "\nSince a HIT was found for previous TP, braggcnt is now:     " << braggcnt << std::endl;
            if (braggcnt==3){
              tp_list_sf = tp_list_next;
              std::cout << "\tThe braggcnt is now 3" << std::endl;
            }

            if (braggcnt >= tracklen/2){
              std::cout << "Now we calculate the slopes" << std::endl;
              frontfound = true;
              int denf = (tp_list_next[imaxadc].channel - tp_list_sf[imaxadc].channel);
              if (denf!=0){
                    frontslope_top = float(tp_list_next[imaxadc].time_start + tp_list_next[imaxadc].time_over_threshold - tp_list_sf[imaxadc].time_start - tp_list_sf[imaxadc].time_over_threshold)/ denf;

                    frontslope_mid = float(tp_list_next[imaxadc].time_start + (tp_list_next[imaxadc].time_over_threshold)/2 -tp_list_sf[imaxadc].time_start - (tp_list_sf[imaxadc].time_over_threshold)/2)/ denf ;

                    frontslope_bottom = float(tp_list_next[imaxadc].time_start - tp_list_sf[imaxadc].time_start)/ denf;

                    std::cout << frontslope_top << std::endl;
                    std::cout << frontslope_mid << std::endl;
                    std::cout << frontslope_bottom << std::endl;
            }
            }
            tp_list_prev = tp_list_next;
          }
        hitfound = false;
        this_time_max = 0;
        this_time_min = 0;
        prev_time_max = 0;
        prev_time_min = 0;
        }

        if(   (tp_list[icheck].channel == (chnl_maxadc+1)) or    (tp_list[icheck].channel == (chnl_maxadc+2)) or    (tp_list[icheck].channel == (chnl_maxadc+3)) or    (tp_list[icheck].channel == (chnl_maxadc+4)) ){
          std::cout << "checking  time condition" << std::endl;
          this_time_max = tp_list[icheck].time_start + tp_list[icheck].time_over_threshold;
          this_time_min =  tp_list[icheck].time_start;
          prev_time_max = tp_list_prev[imaxadc].time_start + tp_list_prev[imaxadc].time_over_threshold;
          prev_time_min =tp_list_prev[imaxadc].time_start;

          std::cout << "this_time_max: " << this_time_max << std::endl;
          std::cout << "this_time_min: " << this_time_min << std::endl;
          std::cout << "prev_time_max: " << prev_time_max << std::endl;
          std::cout << "prev_time_min: " << prev_time_min << std::endl;
          std::cout << ((this_time_min>=prev_time_min and this_time_min<=prev_time_max) or (this_time_max>=prev_time_min and this_time_max<=prev_time_max) or (prev_time_max<=this_time_max and prev_time_min>=this_time_min) ) << std::endl;


          if ((this_time_min>=prev_time_min and this_time_min<=prev_time_max) or (this_time_max>=prev_time_min and this_time_max<=prev_time_max) or (prev_time_max<=this_time_max and prev_time_min>=this_time_min) ){
            std::cout << "\nTime Condition Satisfied --> Contiguous TPs"  << std::endl;
            if (horiz_noise_cnt == 0){
              horiz_tb = prev_time_min;
              horiz_tt = prev_time_max;
            }

            std::cout << "Works" <<std::endl;
            hitfound = true;
            std::cout << "HIT DETECTED\n" << std::endl;
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
        std::cout << "-------------------------------------" << std::endl;
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
        std::cout << "\n\n#####################################" << std::endl;
        std::cout << "BACKWARDS PART OF ALGORITHM ACTIVATED" << std::endl;
        std::cout << "#####################################\n\n" << std::endl;
        for (int icheckb=maxadcindex; icheckb>=0; icheckb--){


            std::cout << "Previous TP";
            std::cout << "\t\t|channel:     " << tp_list_next[imaxadc].channel <<std::endl;
            std::cout << "\t\t\t\t|time_start:  " << tp_list_next[imaxadc].time_start  << "\n" <<std::endl;

          std::cout << "Current TP";
          std::cout << "\t\t|channel:     " << tp_list[icheckb].channel << std::endl;
          std::cout << "\t\t\t\t|time_start:  " << tp_list[icheckb].time_start<< std::endl;


          if(tp_list[icheckb].channel <= (chnl_maxadc-2)){
              std::cout << "lulz" << std::endl;
              chnl_maxadc = tp_list_next[imaxadc].channel;
              if (hitfound == false) break;
              if (hitfound == true) {
                braggcnt+=1;
                std::cout << "\nSince a HIT was found for previous TP, braggcnt is now:     " << braggcnt << std::endl;
                if (braggcnt == (tracklen/2)+3){
                  std::cout << "SB has been updated" <<std::endl;
                  tp_list_sb = tp_list_next;
                }

               if (braggcnt >= tracklen){
                std::cout << "Now we calculate the slopes and angles" << std::endl;
                bky1=tp_list_next[imaxadc].time_start;
                bky2=tp_list_next[imaxadc].time_over_threshold;
                bky3=tp_list_sb[imaxadc].time_start;
                bky4=tp_list_sb[imaxadc].time_over_threshold;

                float num = float(bky1+bky2-(bky3+bky4));
                int den = (tp_list_next[imaxadc].channel - tp_list_sb[imaxadc].channel);
                std::cout << "this is den: " << den << std::endl;
                if(den!=0){
                  backslope_top = (bky1+bky2-(bky3+bky4)) / den;
                  backslope_mid = (bky1+(bky2/2)-(bky3+(bky4/2))) / den;
                  backslope_bottom = (bky1-bky3) / den;

                  std::cout << backslope_top << std::endl;
                  std::cout << backslope_mid << std::endl;
                  std::cout << backslope_bottom << std::endl;

                  frontangle_top = (atan(slopecm_scale*float(frontslope_top)))*radTodeg;
                  backangle_top = (atan(slopecm_scale*float(backslope_top)))*radTodeg;
                  frontangle_mid = (atan(slopecm_scale*float(frontslope_mid)))*radTodeg;
                  backangle_mid = (atan(slopecm_scale*float(backslope_mid)))*radTodeg;
                  frontangle_bottom = (atan(slopecm_scale*float(frontslope_bottom)))*radTodeg;
                  backangle_bottom = (atan(slopecm_scale*float(backslope_bottom)))*radTodeg;

                  std::cout << "frontangle_top:     " << frontangle_top << std::endl;
                  std::cout << "backangle_top:     " << backangle_top << std::endl;
                  std::cout << "frontangle_mid:     " << frontangle_mid << std::endl;
                  std::cout << "backangle_mid:     " << backangle_mid << std::endl;
                  std::cout << "frontangle_bottom:     " << frontangle_bottom << std::endl;
                  std::cout << "backangle_bottom:     " << backangle_bottom << std::endl;
                  std::cout << "angle:    " << (abs(frontangle_top-backangle_top)) << std::endl;
                  std::cout << "Now using Georgia's Method" << std::endl;
                  std::cout << (180 - (abs(frontangle_top-backangle_top))) << std::endl;


                  if (abs(frontangle_mid-backangle_mid)>30 or abs(frontangle_top-backangle_top)>30 or abs(frontangle_bottom-backangle_bottom)>30){
                    trigtot += 1;
                    std::cout <<" Trigger: " << trigtot << std::endl;
                    TriggerPrimitive tp_final {tp_list_maxadc[imaxadc].time_start, tp_list_maxadc[imaxadc].time_peak,  tp_list_maxadc[imaxadc].time_over_threshold, tp_list_maxadc[imaxadc].channel, tp_list_maxadc[imaxadc].adc_integral, tp_list_maxadc[imaxadc].adc_peak, tp_list_maxadc[imaxadc].detid, tp_list_maxadc[imaxadc].type, tp_list_maxadc[imaxadc].algorithm, tp_list_maxadc[imaxadc].version, tp_list_maxadc[imaxadc].flag};
                    std::cout << "\t\tMICHELFOUND" << std::endl;
                    final_tp_list.push_back(tp_final);
                    std::cout <<" Trigger AT (bragg peak found at ): " << tp_list_maxadc[imaxadc].channel << " , " << tp_list_maxadc[imaxadc].time_start << " , "<< tp_list_maxadc[imaxadc].adc_integral << std::endl;
                    break;
                  }
                  else {
                    break;
                  }
                }}

                tp_list_prev = tp_list_next;
              }

            std::cout << "here" << std::endl;
            hitfound = false;
            this_time_max = 0;
            this_time_min = 0;
            prev_time_max = 0;
            prev_time_min = 0;
          }

          if( (tp_list[icheckb].channel == (chnl_maxadc-1)) or  (tp_list[icheckb].channel == (chnl_maxadc-2)) or (tp_list[icheckb].channel == (chnl_maxadc-3)) or (tp_list[icheckb].channel == (chnl_maxadc-4)) ){
            std::cout << "\tChecking if time condition has been satisfied" <<std::endl;
            this_time_max = tp_list[icheckb].time_start + tp_list[icheckb].time_over_threshold;
            this_time_min =  tp_list[icheckb].time_start;
            prev_time_max = tp_list_prev[imaxadc].time_start + tp_list_prev[imaxadc].time_over_threshold;
            prev_time_min =tp_list_prev[imaxadc].time_start;

            if ((this_time_min>=prev_time_min and this_time_min<=prev_time_max) or (this_time_max>=prev_time_min and this_time_max<=prev_time_max) or (prev_time_max<=this_time_max and prev_time_min>=this_time_min) ) {
              std::cout << "\nTime Condition Satisfied --> Contiguous TPs"  << std::endl;
              if (horiz_noise_cnt == 0){
                  horiz_tb = prev_time_min;
                        horiz_tt = prev_time_max;
                }

              hitfound = true;
              std::cout << "\tHIT Detected\n" << std::endl;
              std::cout << tp_list[icheckb].channel << std::endl;
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
        std::cout << "-------------------------------------" << std::endl;
        }
      }
    }


    tp_list.clear();
    tp_list.push_back(input_tp);
  }

}
