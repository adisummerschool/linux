#!/bin/bash
DEVICE="/sys/bus/iio/devices/iio:device0"

echo 0 > "$DEVICE/en"

while true; do
    v0=$(cat "$DEVICE/in_voltage0_raw")
    v1=$(cat "$DEVICE/in_voltage1_raw")
    v2=$(cat "$DEVICE/in_voltage2_raw")
    v3=$(cat "$DEVICE/in_voltage3_raw")
    v4=$(cat "$DEVICE/in_voltage4_raw")
    v5=$(cat "$DEVICE/in_voltage5_raw")

    echo $v0 > "/sys/bus/iio/devices/iio:device2/out_count0_raw"
    echo $v1 > "/sys/bus/iio/devices/iio:device2/out_count1_raw"
    echo $v2 > "/sys/bus/iio/devices/iio:device2/out_count2_raw"
    echo $v3 > "/sys/bus/iio/devices/iio:device2/out_count3_raw"
    echo $v4 > "/sys/bus/iio/devices/iio:device2/out_count4_raw"
    echo $v5 > "/sys/bus/iio/devices/iio:device2/out_count5_raw"

    printf "\rX-: %4d | X+: %4d | Y+: %4d | Y-: %4d | Z+: %4d | Z-: %4d" "$v0" "$v1" "$v2" "$v3" "$v4" "$v5"

    sleep 0.1
done