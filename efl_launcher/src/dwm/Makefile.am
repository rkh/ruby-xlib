AM_CPPFLAGS = -DVERSION=\"${VERSION}\"
AM_CFLAGS = -I. -I/usr/include -I/usr/X11R6/include

bin_PROGRAMS = dwm_edje_wm
dwm_edje_wm_SOURCES = dwm.c config.h
dwm_edje_wm_LDADD = -lpthread -L/usr/lib -lc -L/usr/X11R6/lib -lX11

