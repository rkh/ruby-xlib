#!/bin/bash
xrandr -s 1
xterm &
uxterm &
sleep 10
xeyes &
sleep 2
./wm
