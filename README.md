# e-ink-gauges
C code for e-Paper to display Signal-K data using
9.7inch E-Ink display HAT for Raspberry Pi, 1200×825 resolution

https://www.waveshare.com/wiki/9.7inch_e-Paper_HAT

Curl c library is used to get data from Signal-K server via REST API. If not installed do:

sudo apt install libcurl4-openssl-dev

Set Signal-K server url and 'deltas' in gauges.c lines 109-124

make
run sudo ./myboat</br>
'myboad' is defined as TARGET in Makefile

Tested with Raspberry Pi 4 using sample-nmea0183-data from Signal-K server.
For RPi3/3B/3B+ check line 142 in file lib/DEV_Config.c 

Added '5 minutes to start' countdown timer.
Instead of depth the display presents minutes and second to regatta start (there is usually enough water under starting line).<br>
Timer is started either by pressing button or via Bluetooth connection from paired Android device using RFCOMM app, for instance Arduino Bluetooth controller in Terminal mode.<br>
Button acts connecting shortly purple wire HRDY (8) and brown wire GDN (2) in SPI connector that comes with Pi HAT. </br> Command '5'  from Blutooth terminal starts the timer, 'na' stops.
 
There are several instructions how to make the connection, for instance: <br>
https://iotmyway.wordpress.com/2018/10/24/connecting-raspberry-pi-3-to-android-via-blueterm/

Check these lines in file: 
/etc/systemd/system/dbus-org.bluez.service</br>
[Service]</br>
ExecStart=/usr/lib/bluetooth/bluetoothd -C</br>
ExecStartPost=/usr/bin/sdptool add SP

To start bluetooth connection -> python3 bluecom.py &</br>
in directory where myboat executable is.

![Alt text](/pic/rileygauges.jpg?raw=true "Gauges")
