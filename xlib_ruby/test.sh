#!/bin/bash
xrandr -s 1
xterm &
uxterm &
sleep 10
./wm > wm.log
sleep 2
