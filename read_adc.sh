#!/bin/bash

DEVICE="/sys/bus/iio/devices"

while true; do
    cat $DEVICE/iio:device0/in_voltage0_raw > $DEVICE/iio:device2/out_count0_ra>
    cat $DEVICE/iio:device0/in_voltage1_raw > $DEVICE/iio:device2/out_count1_raw
    cat $DEVICE/iio:device0/in_voltage2_raw > $DEVICE/iio:device2/out_count2_raw
    cat $DEVICE/iio:device0/in_voltage3_raw > $DEVICE/iio:device2/out_count3_raw
    cat $DEVICE/iio:device0/in_voltage4_raw > $DEVICE/iio:device2/out_count4_raw
    cat $DEVICE/iio:device0/in_voltage5_raw > $DEVICE/iio:device2/out_count5_raw
    v0=$(cat "$DEVICE/iio:device0/in_voltage0_raw")
    v1=$(cat "$DEVICE/iio:device0/in_voltage1_raw")
    v2=$(cat "$DEVICE/iio:device0/in_voltage2_raw")
    v3=$(cat "$DEVICE/iio:device0/in_voltage3_raw")
    v4=$(cat "$DEVICE/iio:device0/in_voltage4_raw")
    v5=$(cat "$DEVICE/iio:device0/in_voltage5_raw")

    printf "\rX-: %4d | X+: %4d | Y+: %4d | Y-: %4d | Z+: %4d | Z-: %4d" "$v0" "$v1" "$v2" "$v3" "$v4" "$v5"

    sleep 0.2
done