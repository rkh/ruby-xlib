#!/usr/bin/env ruby
require 'mkmf'

abort 'need stdio.h' unless have_header("stdio.h")
abort 'need pthread.h' unless have_header("pthread.h")

dir_config("Pthreads")
create_makefile("Pthreads")
