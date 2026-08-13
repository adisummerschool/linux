#!/bin/bash

DEVICE="/sys/bus/iio/devices/iio:device0"
LEDS="/sys/bus/iio/devices/iio:device2"

echo 0 > "${DEVICE}/en"
run=true

while $run; do
        ch0=$(cat "${DEVICE}/in_voltage0_raw")
        ch1=$(cat "${DEVICE}/in_voltage1_raw")
        ch2=$(cat "${DEVICE}/in_voltage2_raw")
        ch3=$(cat "${DEVICE}/in_voltage3_raw")
        ch4=$(cat "${DEVICE}/in_voltage4_raw")
        ch5=$(cat "${DEVICE}/in_voltage5_raw")
        if [ $ch5 -gt 2000 ]; then
                run=false
                ch0=0
                ch1=0
                ch2=0
                ch3=0
                ch4=0
                ch5=0
        fi

        echo $ch0 > "${LEDS}/out_count0_raw"
        echo $ch1 > "${LEDS}/out_count1_raw"
        echo $ch2 > "${LEDS}/out_count2_raw"
        echo $ch3 > "${LEDS}/out_count3_raw"
        echo $ch4 > "${LEDS}/out_count4_raw"
        echo $ch5 > "${LEDS}/out_count5_raw"

        sleep 0.1
done