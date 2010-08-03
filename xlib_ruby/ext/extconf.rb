#!/usr/bin/env ruby

require 'mkmf'
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <locale.h>


find_header("X11/Xlib.h", "/usr/X11R6/include");
find_header("X11/Xatom.h", "/usr/X11R6/include");
find_header("X11/Xproto.h", "/usr/X11R6/include");
find_header("X11/Xutil.h", "/usr/X11R6/include");
find_header("X11/keysym.h", "/usr/X11R6/include");
find_header("locale.h", ["/usr/include"]);
find_library("X11", "XOpenDisplay", "/usr/lib", "/usr/X11/lib")

$libs = append_library($libs, "X11")
$libs = append_library($libs, "c")
$libs = append_library($libs, "c")
create_makefile('x11')
