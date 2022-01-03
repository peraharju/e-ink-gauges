# e-ink-gauges
C code for e-Paper to display Signal-K data using
9.7inch E-Ink display HAT for Raspberry Pi, 1200×825 resolution

https://www.waveshare.com/wiki/9.7inch_e-Paper_HAT

Curl c library is used to get data from Signal-K server via REST API. If not installed do:

sudo apt install libcurl4-openssl-dev

Set displayed values in gauges.c file lines 94-97

make

Give Signal-K server url as argument for runnable main method</br>
run sudo ./myboat http://192.168.1.111:3000</br>
'myboad' is defined as TARGET in Makefile

Tested with Raspberry Pi 4 and Zero 2W using sample-nmea0183-data from Signal-K server.
For RPi3/3B/3B+ check line 142 in file lib/Config/DEV_Config.c </br>
bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);   //For RPi 4 and Zero 2 </br>
Current consumption of Pi Zero with display updating is about 190 mA, less than half of RPI 4.

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
