#!/bin/bash
#
# Show subnet traffic. Helpful to see network load during a game
# and figure out which team is responsible.

ipband wlan0 -m 24 -C -d 2 -r 1 -a 1 -t 5 -b 0 -L 192.168.0.0/16
