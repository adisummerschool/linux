// SPDX-License-Identifier: GPL-2.0
/*
 * AD7170/AD7171 and AD7780/AD7781 SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2019 Renato Lui Geh
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

const int CHANNEL_0 = 0;
const int CHANNEL_1 = 1;
const int CHANNEL_2 = 2;
const int CHANNEL_3 = 3;
const int CHANNEL_4 = 4;
const int CHANNEL_5 = 5;

static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
{
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if(chan->channel == CHANNEL_0) 
                *val = 0;
            else if(chan->channel == CHANNEL_1) 
                *val = 1;
                else if(chan->channel == CHANNEL_2)
                    *val = 2;
                    else if(chan->channel == CHANNEL_3)
                        *val = 3;
                        else if(chan->channel == CHANNEL_4)
                            *val = 4;
                            else if(chan->channel == CHANNEL_5)
                                *val = 5;    
            return IIO_VAL_INT;                
        default: 
            return -EINVAL;
    }
}

static int iio_ad5592r_s_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int val, int val2, long mask)
{
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if(chan->channel == CHANNEL_0) 
                dev_info(&indio_dev->dev, "Trying to write to channel 0: %d", val);
            else  if(chan->channel == CHANNEL_1) 
                    dev_info(&indio_dev->dev, "Trying to write to channel 1: %d", val);
                else  if(chan->channel == CHANNEL_2)
                        dev_info(&indio_dev->dev, "Trying to write to channel 2: %d", val);  
                    else if(chan->channel == CHANNEL_3)
                            dev_info(&indio_dev->dev, "Trying to write to channel 3: %d", val);
                        else if(chan->channel == CHANNEL_4)
                                dev_info(&indio_dev->dev, "Trying to write to channel 4: %d", val); 
                            else if(chan->channel == CHANNEL_5)
                                    dev_info(&indio_dev->dev, "Trying to write to channel 5: %d", val); 
            return 0;                
        default: 
            return -EINVAL;
    }
}
static const struct iio_chan_spec iio_ad5592r_s_channels[] = {
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_0,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_1,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_2,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_3,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_4,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_5,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW)
    }
};


static const struct iio_info iio_ad5592r_s_info = {
    .read_raw = &iio_ad5592r_s_read_raw,
    .write_raw = &iio_ad5592r_s_write_raw
};

// probe function
static int iio_ad5592r_s_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;
    indio_dev = devm_iio_device_alloc(&spi->dev,0);
    indio_dev->name = "ad5592r_s";
    indio_dev->info = &iio_ad5592r_s_info;
     indio_dev->channels = iio_ad5592r_s_channels;
    indio_dev->num_channels = 6;

    return devm_iio_device_register(&spi->dev, indio_dev);
} 


static struct spi_driver iio_ad5592r_s_driver = {
    .driver = {
        .name = "ad5592r_s"
    },
    .probe = iio_ad5592r_s_probe
};
module_spi_driver(iio_ad5592r_s_driver);


MODULE_AUTHOR("Mascasan Maya-Lorena <mascasan.se.maya@student.utcluj.ro>");
MODULE_DESCRIPTION("Analog Devices - ADC Driver");
MODULE_LICENSE("GPL v2");