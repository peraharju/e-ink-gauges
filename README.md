# e-ink-gauges
C code for e-Paper to display Signal-K data using
9.7inch E-Ink display HAT for Raspberry Pi, 1200×825 resolution

https://www.waveshare.com/wiki/9.7inch_e-Paper_HAT

Curl c library is used to get data from Signal-K server via REST API. If not installed do:
sudo apt install libcurl4-openssl-dev

set Signal-K server url and 'deltas' in gauges.c lines 100-112

make
run sudo ./myboat 

'myboad' is defined as TARGET in Makefile

Tested with Raspberry Pi 4 using sample-nmea0183-data from Signal-K server.
For RPi3/3B/3B+ check line 142 in file lib/DEV_Config.c 
![Alt text](/pic/rileygauges.jpg?raw=true "Gauges")
