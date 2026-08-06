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

struct iio_adc_emu_st {
    int reg_select;
    int chan_val[2];
};

static int iio_adc_emu_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
{
    struct iio_adc_emu_st *st = iio_priv(indio_dev);
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if(!st->reg_select){
                if(chan->channel) 
                    *val = st->chan_val[1];
                else 
                    *val = st->chan_val[0];
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

static int iio_adc_emu_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int val, int val2, long mask)
{
     struct iio_adc_emu_st *st = iio_priv(indio_dev);
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if(!st->reg_select){
                if(chan->channel){ 
                    dev_info(&indio_dev->dev, "Trying to write to channel 1");
                    st->chan_val[1] = val;
                }
                else{
                    dev_info(&indio_dev->dev, "Trying to write to channel 2");
                    st->chan_val[0] = val;
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
static const struct iio_chan_spec iio_adc_emu_channels[] = {
    {
        .type = IIO_VOLTAGE,
        .channel = 0,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = 1,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    }
};

static const struct iio_info iio_adc_emu_info = {
    .read_raw = &iio_adc_emu_read_raw,
    .write_raw = &iio_adc_emu_write_raw
};

// probe function
static int iio_adc_emu_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;
    struct iio_adc_emu_st *st;
    indio_dev = devm_iio_device_alloc(&spi->dev,sizeof(*st));
    st = iio_priv(indio_dev);
    st->reg_select = 1;
    memset(st->chan_val, 0, sizeof(st->chan_val));
    indio_dev->name = "iio_adc_emu";
    indio_dev->info = &iio_adc_emu_info;
    indio_dev->channels = iio_adc_emu_channels;
    indio_dev->num_channels = ARRAY_SIZE(iio_adc_emu_channels);

    return devm_iio_device_register(&spi->dev, indio_dev);
} 

static struct spi_driver iio_adc_emu_driver = {
    .driver = {
        .name = "iio_adc_emu"
    },
    .probe = iio_adc_emu_probe
};
module_spi_driver(iio_adc_emu_driver);

MODULE_AUTHOR("Mascasan Maya-Lorena <mascasan.se.maya@student.utcluj.ro>");
MODULE_DESCRIPTION("Analog Devices - ADC EMU Driver");
MODULE_LICENSE("GPL v2");