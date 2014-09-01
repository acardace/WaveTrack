WaveTrack
=========

A real-time pitch-tracking library based on Wavelets.

It's an optimized implementation of the algorithm described in this paper http://physics.illinois.edu/undergrad/reu/2005/Larson.Maddox.pdf.

There is an example file pitchTracker.c showing how the library can be used using ALSA, however any kind of sound acquisition method (Pulse,OSS,JACK..) can be used.

WaveTrack has been developed in C and is capable of achieve great performance both in accuracy and in speed, as a matter of fact when using a buffer of 2048 samples (S16) the latency is of ~21ms, which is just the latency brought by ALSA, because the library usually generates approximately ~0.1ms of latency.

WaveTrack works best with frequencies ranging form 80 Hz to 3000 Hz.

Here's how you can compile the example file:

gcc -O3 wavetrack.c pitchTracker.c -o pitchTracker -lasound -lm
