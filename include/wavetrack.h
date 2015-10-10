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

/*Set of defitions */
#ifndef _wavelet_tracker__
#define _wavelet_tracker__

#define UNPITCHED 0.0
#define NONE      -1

#define SAMPLE_RATE 44100.0
#define FLWT_LEVELS 6
#define DIFFS_LEVELS 3
#define MAX_FREQ 3000.0
#define THRESHOLD_RATIO 0.75

#define SILENCE_THRESHOLD 0.7

/* ring buffer operation macros */
#define RINGSIZE  3 //dimension of the ring buffer, a bigger buffer has more precision in returned values, but less reactive over short (duration) notes
                    // a smaller one gives more reactivity  but less accuracy, when operating in SPEED mode, the suggested size is 3, when in ACCURACY mode is 2
#define PRECISION 5 //precision in Hz

/* Algorithm operating mode parameter*/
#define ACCURACY  0
#define SPEED     1

//sample formats
#define S8  256/2*SILENCE_THRESHOLD
#define U8  256*SILENCE_THRESHOLD
#define S16 65536/2*SILENCE_THRESHOLD
#define U16 65536*SILENCE_THRESHOLD
#define S24 16777216/2*SILENCE_THRESHOLD
#define U24 16777216*SILENCE_THRESHOLD
#define S32 4294967296/2*SILENCE_THRESHOLD
#define U32 4294967296*SILENCE_THRESHOLD

/* init function which sets the algorithm standard parameters, to be called before compute_pich */
struct pitch_tracker_params* open_pitch_tracker();

/* to be called when the library is no longer to be used */
void close_pitch_tracker(struct pitch_tracker_params **settings);

/* call to compute the pitch of the current window */
double compute_pitch(double *sample_vector, unsigned int samplecount, unsigned int sample_format, struct pitch_tracker_params *settings, int op_mode);

/* setter functions */
void tracker_set_maxfreq(struct pitch_tracker_params *settings, double freq);
void tracker_set_flwtlevels(struct pitch_tracker_params *settings, unsigned int levels);
void tracker_set_difflevels(struct pitch_tracker_params *settings, unsigned int levels);
void tracker_set_thresholdratio(struct pitch_tracker_params *settings, double ratio);

/* structure containing the algorithm parameters */
struct pitch_tracker_params;

/* utility functions */
static inline int abs_val(double num){ return num >= 0. ? num : -num; }

#ifndef max
#define max(x, y) ((x) > (y)) ? x : y
#endif

#ifndef min
#define min(x, y) ((x) < (y)) ? x : y
#endif


#endif

