filesdir = $(datadir)/dwm_edje/

files_DATA = Edjelist.rb Edjewin.rb Launcher.rb cruby/Pthreads.so desktop-menu.rb 
files_SCRIPTS = Launcher.rb
CLEANFILES = Launcher.rb

Launcher.rb:
	sed 's#(DATADIR)#$(datadir)#g' launcher_bare.rb > Launcher.rb
