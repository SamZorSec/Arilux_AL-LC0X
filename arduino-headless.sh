#!/bin/bash
killall -9 Xvfb
Xvfb :1 -nolisten tcp -screen :1 1280x800x24 &
xvfb="$!"
sleep 1s
DISPLAY=:1 arduino "$@"
kill -9 $xvfb
