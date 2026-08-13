#!/bin/sh

IIO_PATH=/sys/bus/iio/devices   

mkdir /configfs;
mount -t configfs none configfs;
systemctl daemon-reload;
mkdir /configfs/iio/triggers/hrtimer/tmr0;
echo 0 > ${IIO_PATH}/iio\:device1/en;
echo tmr0 > ${IIO_PATH}/iio\:device1/trigger/current_trigger
echo 0.1 > ${IIO_PATH}trigger0/sampling_frequency