#!/bin/bash
IIO_PATH=/sys/bus/iio/devices

mkdir configfs;
mount -t configfs none configfs;
mkdir configfs/iio/triggers/hrtimer/tmr0;
systemctl daemon-reload;
cat /sys/bus/iio/devices/trigger0/name;
echo 1 > ${IIO_PATH}/iio\:device0/en;
echo tmr0 > ${IIO_PATH}/iio\:device0/trigger/current_trigger;
echo 0.1 > ${IIO_PATH}/trigger0/sampling_frequency;