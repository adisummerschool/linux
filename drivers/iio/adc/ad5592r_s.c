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

struct iio_adc_emu_st {
    int reg_select;
    int chan_val[6];
};

static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
{
    struct iio_adc_emu_st *st = iio_priv(indio_dev);
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if (!st->reg_select) {
                if(chan->channel == CHANNEL_0) 
                    *val = st->chan_val[0];
                else if(chan->channel == CHANNEL_1) 
                    *val = st->chan_val[1];
                    else if(chan->channel == CHANNEL_2)
                        *val = st->chan_val[2];
                        else if(chan->channel == CHANNEL_3)
                            *val = st->chan_val[3];
                            else if(chan->channel == CHANNEL_4)
                                *val = st->chan_val[4];
                                else if(chan->channel == CHANNEL_5)
                                    *val = st->chan_val[5];
                return IIO_VAL_INT; 
            }
            else 
                return -EINVAL;               
        case IIO_CHAN_INFO_ENABLE:
            *val = st->reg_select;
            return IIO_VAL_INT;
        default: 
            return -EINVAL;
    }
}

static int iio_ad5592r_s_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int val, int val2, long mask)
{
    struct iio_adc_emu_st *st = iio_priv(indio_dev);
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if(!st->reg_select){
                if(chan->channel == CHANNEL_0){
                    dev_info(&indio_dev->dev, "Trying to write to channel 0: %d", val);
                    st->chan_val[0] = val;
                }
                else  if(chan->channel == CHANNEL_1){
                        dev_info(&indio_dev->dev, "Trying to write to channel 1: %d", val);
                        st->chan_val[1] = val;
                    }
                    else  if(chan->channel == CHANNEL_2){
                            dev_info(&indio_dev->dev, "Trying to write to channel 2: %d", val); 
                            st->chan_val[2] = val; 
                    }
                        else if(chan->channel == CHANNEL_3){
                                dev_info(&indio_dev->dev, "Trying to write to channel 3: %d", val);
                                st->chan_val[3] = val;
                        }
                            else if(chan->channel == CHANNEL_4){
                                    dev_info(&indio_dev->dev, "Trying to write to channel 4: %d", val); 
                                    st->chan_val[4] = val;
                            }
                                else if(chan->channel == CHANNEL_5){
                                        dev_info(&indio_dev->dev, "Trying to write to channel 5: %d", val); 
                                        st->chan_val[5] = val;
                                }
                return 0;
            }
            else
                return -EINVAL;  
        case IIO_CHAN_INFO_ENABLE:
            st->reg_select = val ? 1 : 0;
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
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_1,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_2,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_3,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_4,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_5,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    }
};


static const struct iio_info iio_ad5592r_s_info = {
    .read_raw = &iio_ad5592r_s_read_raw,
    .write_raw = &iio_ad5592r_s_write_raw
};

// probe function
static int iio_ad5592r_s_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;
    struct iio_adc_emu_st *st;
    indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
    st = iio_priv(indio_dev);
    st->reg_select = 1;
    memset(st->chan_val, 0, sizeof(st->chan_val));
    indio_dev->name = "ad5592r_s";
    indio_dev->info = &iio_ad5592r_s_info;
     indio_dev->channels = iio_ad5592r_s_channels;
    indio_dev->num_channels = ARRAY_SIZE(iio_ad5592r_s_channels);

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