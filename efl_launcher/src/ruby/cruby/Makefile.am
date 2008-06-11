filesdir = $(datadir)/dwm_edje/

files_DATA = Pthreads.so
files_SCRIPTS = Pthreads.so
CLEANFILES = mkmf.log Pthreads.so

Pthreads.so:
	cp Makefile Makefile.or && ruby extconf.rb && make && mv Makefile.or Makefile
