/*
 * Wavetrack, a pitch tracking library using wavelets.
 * Copyright (C) 2014  Antonio Cardace.
 *
 * This file is part of Wavetrack.
 *
 * Wavetrack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Wavetrack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<wavetrack.h>
#include<string.h>

/* definition of the struct containing the algorithm parameters */
struct pitch_tracker_params{
   unsigned int diff_levels;
   unsigned int flwt_levels;
   unsigned int max_freq;
   double max_threshold_ratio;
};

/* functions to set the algorithm parameters */
struct pitch_tracker_params* open_pitch_tracker(){
  struct pitch_tracker_params *settings = (struct pitch_tracker_params *) malloc( sizeof(struct pitch_tracker_params) );

  /*default settings*/
  if(settings != NULL ){
     settings->flwt_levels = FLWT_LEVELS;
     settings->max_freq = MAX_FREQ;
     settings->diff_levels = DIFFS_LEVELS;
     settings->max_threshold_ratio = THRESHOLD_RATIO;
  }

  return settings;
}

/* function to free the allocated struct */
void close_pitch_tracker(struct pitch_tracker_params **settings){
   free(*settings);
}

/* setter functions */
void tracker_set_maxfreq(struct pitch_tracker_params *settings, double freq){
   if( settings != NULL )
      settings->max_freq = freq;
}

void tracker_set_flwtlevels(struct pitch_tracker_params *settings, unsigned int levels){
   if( settings != NULL )
      settings->flwt_levels = levels;
}

void tracker_set_difflevels(struct pitch_tracker_params *settings, unsigned int levels){
   if( settings != NULL )
      settings->diff_levels = levels;
}

void tracker_set_thresholdratio(struct pitch_tracker_params *settings, double ratio){
   if( settings != NULL )
      settings->max_threshold_ratio = ratio;
}

/* Algorithm implementation */
static double __compute_pitch(double *approx,unsigned int samplecount, unsigned int sample_format, struct pitch_tracker_params *settings){
   double pitch = UNPITCHED, mean=0.0, max_freq, max_threshold_ratio, mode=0.0, old_mode_value=0.;
   double max_threshold, min_threshold, DC_component = 0., max_value = 0. ,min_value = 0. , mean_no=0;
   double *mins, *maxs;
   int *distances, dist_delta;
   unsigned int curr_sample_number = samplecount, curr_level;
   unsigned int max_flwt_levels, diff_levels, power2 = 1;//power2 is just to calculate powers of 2 as to speed evertything up
   unsigned int d_index, max_d_index;
   register unsigned int count, center_mode_i, center_mode_c; // centermode index and centermode count
   register unsigned int mins_no, maxs_no, zero_crossed;
   register int too_close, sign_test, prev_sign_test= 0, j, i;
   static double last_mode = UNPITCHED;
   static int last_iter = NONE;

   //Algorithm parameters
   max_flwt_levels = settings->flwt_levels;
   max_freq = settings->max_freq;
   diff_levels = settings->diff_levels;
   max_threshold_ratio = settings->max_threshold_ratio;

   /* compute amplitude Threshold and the DC component */
   for( i = 0 ; i< samplecount ; i++){
      DC_component += approx[i];

      max_value = max( approx[i], max_value );
      min_value = min( approx[i], min_value );
   }


   //as to distinguish silence
   if( sample_format && max_value < sample_format ){
      last_mode = UNPITCHED;
      last_iter = NONE;
      return UNPITCHED;
   }

   DC_component = DC_component/samplecount;
   max_threshold = (max_value - DC_component)*max_threshold_ratio + DC_component;
   min_threshold = (min_value - DC_component)*max_threshold_ratio + DC_component;

   /* init data structures */
   maxs = (double *) malloc( sizeof(double) * ( curr_sample_number/2 ) );
   mins = (double *) malloc( sizeof(double) * (curr_sample_number/2 ) );
   distances = (int *) malloc( sizeof(int) * (curr_sample_number/2 ) );

   for( curr_level=1; curr_level < max_flwt_levels ; curr_level++ ){

      /* set data as to compute the FLWT */
      mins_no = 0;
      maxs_no = 0;
      power2<<=1;

      /* perform the FLWT */
      curr_sample_number /= 2;
      for(j=0; j<=curr_sample_number ; j++){
         approx[j] = ( approx[2*j] + approx[2*j + 1] )/2;
      }

      /*now store the first maxima and minima after each zero-crossing, only store them if they respect the delta rule (above)
        and are greater than the minimum threshold ( amplitude_threshold) */
      dist_delta = (int) max( floor( SAMPLE_RATE/ ( max_freq * power2 ) ), 1);

      //checks if the wave is going up or down, =1 if positive, =-1 if negative
      if( approx[1] - approx[0] > 0 )
         prev_sign_test = 1;
      else
         prev_sign_test = -1;

      zero_crossed = 1; //zero crossing test
      too_close = 0; // keep tracks of how many samples must not be taken into considerations ( max/min finding ) because of the delta

      for( j=1 ; j< curr_sample_number ; j++ ){
         sign_test = approx[j] - approx[j-1];

         if( prev_sign_test >= 0 && sign_test < 0 ){
            if( approx[j-1] >= max_threshold && zero_crossed && !too_close){
               maxs[maxs_no] = j-1;
               maxs_no++;
               zero_crossed = 0;
               too_close = dist_delta;
            }
         }
         else if( prev_sign_test <=0 && sign_test>0 ){
            if( approx[j-1] <= min_threshold && zero_crossed && !too_close ){
               mins[mins_no] = j-1;
               mins_no++;
               zero_crossed = 0;
               too_close = dist_delta;
            }
         }

         if( ( approx[j] <= DC_component && approx[j-1] > DC_component ) || ( approx[j] >= DC_component && approx[j-1] < DC_component ) )
            zero_crossed = 1;

         prev_sign_test = sign_test;

         if(too_close)
            too_close--;

      }

      /* determine the mode distance between the maxima/minima */
      if( maxs_no ||  mins_no ){

         memset( distances, 0, sizeof(int) * curr_sample_number );

         max_d_index = 0; //useful for keeping track of the maximum used index of the array
         d_index = 0;

         for( j=0; j< maxs_no ; j++)
            for( i=1 ; i<= diff_levels ; i++ )
               if( i+j < maxs_no ){
                  d_index = abs_val( maxs[j] - maxs[i+j] );
                  distances[d_index]++;
                  if( d_index > max_d_index )
                     max_d_index = d_index;
               }

         for( j=0; j< mins_no ; j++)
            for( i=1 ; i<= diff_levels ; i++ )
               if( i+j < mins_no ){
                  d_index = abs_val( mins[j] - mins[i+j] );
                  distances[d_index]++;
                  if( d_index > max_d_index )
                     max_d_index = d_index;
               }

         center_mode_c = 1;
         center_mode_i = 0;

         /* select center mode */
         for( i = 0 ; i <= max_d_index ; i++ ){

            if( distances[i] == 0 ) //useless to go on if this distance is not part of the set of real calculated distances
               continue;

            count = 0;

            for( j = -dist_delta ; j <= dist_delta ; j++){
               if( i+j >= 0 && i+j <= max_d_index )
                  count += distances[i+j];
            }

            if( count == center_mode_c && count > floor(curr_sample_number/i/4) ){
               if( last_mode != UNPITCHED && abs_val( i - last_mode/power2 ) <= dist_delta ){
                  center_mode_i = i;
               }
               else if( i== center_mode_i*2 ){
                  center_mode_i = i;
               }

            }
            else if( count > center_mode_c ){
               center_mode_i = i;
               center_mode_c = count;
            }
            else if( count == center_mode_c-1 && last_mode > UNPITCHED && abs_val( i - last_mode/power2 ) <= dist_delta )
               center_mode_i = i;

         }

         /* mode averaging*/
         mean_no = 0;
         mean = 0;

         if( center_mode_i > 0 ){
            for( i= -dist_delta ; i <= dist_delta ; i++){
               if( center_mode_i + i >= 0 && center_mode_i + i <= max_d_index ){
                  if( distances[center_mode_i+i] == 0 )
                     continue;

                  mean_no += distances[center_mode_i+i];
                  mean += (center_mode_i+i) * distances[center_mode_i+i];
               }

            }
            mode = mean / mean_no;
         }
      }else if( maxs_no == 0 && mins_no == 0){
         free(maxs);
         free(mins);
         free(distances);
         last_mode = UNPITCHED;
         last_iter = NONE;
         return UNPITCHED;
      }

      /* letś see if we can see some underlying periodicity */


      /* if the mode distance is equivalent to that of the previous level, then is taken as the period, otherwise next level of FLWT */
       if( old_mode_value>0.  && abs_val( 2*mode - old_mode_value ) <= dist_delta  ){
         free(maxs);
         free(mins);
         free(distances);

         pitch = SAMPLE_RATE/( power2/2*old_mode_value );
         last_iter = curr_level-2;
         last_mode = mode;
         return pitch;
       }
       /* if the mode from the previous windows is similar to the one computed in the current window, then it is the same frequency */
       else if( last_mode > 0. && curr_level == 1 && last_iter>-1 && ( abs_val( ( last_mode*( power2<<last_iter)) - mode ) ) <= dist_delta ){
          free(maxs);
          free(mins);
          free(distances);

          pitch = SAMPLE_RATE/( power2*mode );
          last_mode = mode;
          last_iter = 1;
          return pitch;
       }

       /*set oldmode for next iteration*/
       old_mode_value = mode;
   }

   // No pitch detected
   free(maxs);
   free(mins);
   free(distances);
   last_mode = UNPITCHED;
   last_iter = NONE;
   return UNPITCHED;
}


/* When op_mode is set to ACCURACY the algorithm mainly targets precision of the returned value over speed, alternatively
 * when set to SPEED it targets speed over precision so more values are returned but they might be less accurate */
double compute_pitch(double *approx,unsigned int samplecount, unsigned int sample_format, struct pitch_tracker_params *settings, int op_mode){
   static double ring[RINGSIZE];
   static unsigned int i = 0;
   double pitch, precision, best_precision = 50.;
   int j;

   pitch =  __compute_pitch( approx, samplecount, sample_format, settings);
   ring[i] = pitch;
   i= (i+1) % RINGSIZE;

   if( pitch != UNPITCHED ){
      for(j=0 ; j<RINGSIZE ; j++){
         precision = abs_val( ring[j] - ring[(j+1) % RINGSIZE]);
         if( precision < best_precision ){
            pitch = ( ring[j] + ring[(j+1) % RINGSIZE] )/2;
            best_precision = precision;
         }
      }
   }

   if( best_precision <= PRECISION )
      return pitch;

   if( op_mode == SPEED )
      return pitch;
   else
      return UNPITCHED;
};
