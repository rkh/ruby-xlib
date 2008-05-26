# Copyright (C) 2008 Tim Felgentreff <timfelgentreff@gmail.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

filesdir = $(prefix)
bin_PROGRAMS = dwm_edje_launcher

dwm_edje_launcher:
	sed 's#(DATADIR)#$(datadir)#g' startscript > dwm_edje_launcher
	chmod +x dwm_edje_launcher
