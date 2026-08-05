// SPDX-License-Identifier: GPL-2.0
/*
 * AD5592r SPI ADC driver emulator
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2019 Renato Lui Geh
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>


static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,
			struct iio_chan_spec const *chan,
			int *val,
			int *val2,
			long mask)
{
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if (chan->channel)
                *val = 67;
            else 
                *val = 76;
            return IIO_VAL_INT;
        default:
            return -EINVAL;
    }
}

static int iio_adc_emu_write_raw(struct iio_dev *indio_dev,
			 struct iio_chan_spec const *chan,
			 int val,
			 int val2,
			 long mask)
{
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            if (chan->channel)
                dev_info(&indio_dev->dev,"Trying to write to chanel 0");
            else 
                dev_info(&indio_dev->dev,"Trying to write to chanel 1");
            return 0;
        default:
            return -EINVAL;
    }
}

static const struct iio_chan_spec iio_adc_emu_chanels[] = {
    {
        .type = IIO_VOLTAGE,
        .channel = 0,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
    {
        .type = IIO_VOLTAGE,
        .channel = 1,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    }
};

static const struct iio_info iio_adc_emu_info = {
    .read_raw = &iio_adc_emu_read_raw,
    .write_raw = &iio_adc_emu_write_raw,
};


static int iio_adc_emu_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;
    
    indio_dev = devm_iio_device_alloc(&spi->dev,0);
    
    indio_dev->name ="iio_adc_emu";
    indio_dev->info = &iio_adc_emu_info;

    indio_dev->channels = iio_adc_emu_chanels;
    indio_dev->num_channels = 2;
    
    return devm_iio_device_register(&spi->dev,indio_dev);

}

struct spi_driver iio_adc_emu_driver = {
    .driver = {
        .name = "iio_adc_emu"
    },
    .probe = iio_adc_emu_probe
};
module_spi_driver(iio_adc_emu_driver);


MODULE_AUTHOR("Papp Richard <pappr805@gmail.com>");
MODULE_DESCRIPTION("ADC driver emulator");
MODULE_LICENSE("GPL v2");