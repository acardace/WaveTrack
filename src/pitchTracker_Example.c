#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdint.h>
#include<signal.h>
#include<time.h>
#include<unistd.h>
#include<math.h>
#include<wavetrack.h>
#include<alsa/asoundlib.h>

#define BUFSIZE 2048
#define PERIOD_SIZE BUFSIZE/2

int main(int argc, char **argv){
   int i;
   int aver,val,val2;
   int16_t buf[BUFSIZE];
   double d_buffer[BUFSIZE];
   double pitch = 0.0;
   struct timespec before,after;
   struct pitch_tracker_params *settings;
   snd_pcm_uframes_t period_size = PERIOD_SIZE;

   //ALSA PCM configuration
   snd_pcm_t *handle;
   snd_pcm_hw_params_t *params;
   int dir,rc;
   snd_pcm_uframes_t frames;

   /* Open PCM device for capture */
   rc = snd_pcm_open( &handle, "default" , SND_PCM_STREAM_CAPTURE , 0);

   if(rc < 0 ){
      fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));
      exit(1);
   }

   /* Allocate a hardware parameters object. */
   snd_pcm_hw_params_alloca(&params);

   /* Fill it in with default values. */
   snd_pcm_hw_params_any(handle, params);

   /* Set the desired hardware parameters. */

   /* Interleaved mode */
   snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);

   /* unsigned 16-bit format */
   snd_pcm_hw_params_set_format(handle, params,SND_PCM_FORMAT_S16);

   snd_pcm_hw_params_set_channels(handle, params, 1);

   /* 44100 bits/second sampling rate */
   val = 44100;
   snd_pcm_hw_params_set_rate_near(handle,params, &val, &dir);

   /* set size time*/
   snd_pcm_hw_params_set_period_size_near(handle, params, &period_size , &dir);

   /* write configuration */
   rc = snd_pcm_hw_params(handle,params);


   /*Display info*/

   printf("PCM handle name = '%s'\n",
         snd_pcm_name(handle));

   printf("PCM state = %s\n",
         snd_pcm_state_name(snd_pcm_state(handle)));

   snd_pcm_hw_params_get_access(params,
         (snd_pcm_access_t *) &val);
   printf("access type = %s\n",
         snd_pcm_access_name((snd_pcm_access_t)val));

   snd_pcm_hw_params_get_format(params, &val);
   printf("format = '%s' (%s)\n",
         snd_pcm_format_name((snd_pcm_format_t)val),
         snd_pcm_format_description(
            (snd_pcm_format_t)val));

   snd_pcm_hw_params_get_subformat(params,
         (snd_pcm_subformat_t *)&val);
   printf("subformat = '%s' (%s)\n",
         snd_pcm_subformat_name((snd_pcm_subformat_t)val),
         snd_pcm_subformat_description(
            (snd_pcm_subformat_t)val));

   snd_pcm_hw_params_get_channels(params, &val);
   printf("channels = %d\n", val);

   snd_pcm_hw_params_get_rate(params, &val, &dir);
   printf("rate = %d bps\n", val);

   snd_pcm_hw_params_get_period_time(params,
         &val, &dir);
   printf("period time = %d us\n", val);

   snd_pcm_hw_params_get_period_size(params,
         &frames, &dir);
   printf("period size = %d frames\n", (int)frames);

   snd_pcm_hw_params_get_buffer_time(params,
         &val, &dir);
   printf("buffer time = %d us\n", val);

   snd_pcm_hw_params_get_buffer_size(params,
         (snd_pcm_uframes_t *) &val);
   printf("buffer size = %d frames\n", val);

   snd_pcm_hw_params_get_periods(params, &val, &dir);
   printf("periods per buffer = %d frames\n", val);

   snd_pcm_hw_params_get_rate_numden(params,
         &val, &val2);
   printf("exact rate = %d/%d bps\n", val, val2);

   val = snd_pcm_hw_params_get_sbits(params);
   printf("significant bits = %d\n", val);

   val = snd_pcm_hw_params_is_batch(params);
   printf("is batch = %d\n", val);

   val = snd_pcm_hw_params_is_block_transfer(params);
   printf("is block transfer = %d\n", val);

   val = snd_pcm_hw_params_is_double(params);
   printf("is double = %d\n", val);

   val = snd_pcm_hw_params_is_half_duplex(params);
   printf("is half duplex = %d\n", val);

   val = snd_pcm_hw_params_is_joint_duplex(params);
   printf("is joint duplex = %d\n", val);

   val = snd_pcm_hw_params_can_overrange(params);
   printf("can overrange = %d\n", val);

   val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
   printf("can mmap = %d\n", val);

   val = snd_pcm_hw_params_can_pause(params);
   printf("can pause = %d\n", val);

   val = snd_pcm_hw_params_can_resume(params);
   printf("can resume = %d\n", val);

   val = snd_pcm_hw_params_can_sync_start(params);
   printf("can sync start = %d\n", val);

   settings = open_pitch_tracker();

   while(1){
      rc = snd_pcm_readi(handle, buf, frames);
      if (rc == -EPIPE) {
         /* EPIPE means overrun */
         fprintf(stderr, "overrun occurred\n");
         snd_pcm_prepare(handle);
      } else if (rc < 0) {
         fprintf(stderr,
               "error from read: %s\n",
               snd_strerror(rc));
      } else if (rc != (int)frames) {
         fprintf(stderr, "short read, read %d frames\n", rc);
      }


      for( i = 0 ; i< BUFSIZE ; i++){
         d_buffer[i] = (double) buf[i];
      }
      pitch = compute_pitch(d_buffer, BUFSIZE, S16, settings ,ACCURACY);
      if( pitch != 0.0 )
         printf("frequency -> %f\n",pitch);

      memset(buf,0,BUFSIZE);
   }


   close_pitch_tracker(&settings);
   snd_pcm_drain(handle);
   snd_pcm_close(handle);

   return 0;
}
