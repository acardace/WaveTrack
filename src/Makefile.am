AM_CFLAGS = -I$(srcdir)/../include

noinst_PROGRAMS =
if WANT_EXAMPLE
   noinst_PROGRAMS += pitchTracker
endif

pitchTracker_SOURCES = pitchTracker_Example.c
pitchTracker_LDADD = $(srcdir)/../lib/libwavetrack.la
pitchTracker_LDFLAGS = -lasound
