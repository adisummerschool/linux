// SPDX-License-Identifier: GPL-2.0
/*
 * AD5592R SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2026 Papp Richard
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

const int CHANEL_0 = 0, CHANEL_1 = 1, CHANEL_2 = 2;
const int CHANEL_3 = 3, CHANEL_4 = 4, CHANEL_5 = 5;


static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev,
			struct iio_chan_spec const *chan,
			int *val,
			int *val2,
			long mask)
{
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            switch(chan->channel){
                case CHANEL_0:     
                    *val = 67;
                    break;
                case CHANEL_1:
                    *val = 76;
                    break;
                case CHANEL_2:   
                    *val = 420;
                    break;
                case CHANEL_3:    
                    *val = 69;
                    break;
                case CHANEL_4: 
                    *val = 96;
                    break;
                case CHANEL_5:  
                    *val = 42067;
                    break;
                default: 
                    return -EINVAL;
            }
            return IIO_VAL_INT;
        default:
            return -EINVAL;
    }
}

static int iio_ad5592r_s_write_raw(struct iio_dev *indio_dev,
			 struct iio_chan_spec const *chan,
			 int val,
			 int val2,
			 long mask)
{
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            switch(chan->channel){
                case CHANEL_0:     
                    dev_info(&indio_dev->dev,"Trying to write to chanel 0");
                    break;
                case CHANEL_1:
                    dev_info(&indio_dev->dev,"Trying to write to chanel 1");
                    break;
                case CHANEL_2:   
                    dev_info(&indio_dev->dev,"Trying to write to chanel 2");
                    break;
                case CHANEL_3:    
                    dev_info(&indio_dev->dev,"Trying to write to chanel 3");
                    break;
                case CHANEL_4: 
                    dev_info(&indio_dev->dev,"Trying to write to chanel 4");
                    break;
                case CHANEL_5:  
                    dev_info(&indio_dev->dev,"Trying to write to chanel 5");
                    break;
                default: 
                    return -EINVAL;
            }
            return 0;
        default:
            return -EINVAL;
    }
}

static const struct iio_chan_spec iio_ad5592r_s_chanels[] = {
    {
        .type = IIO_VOLTAGE,
        .channel = CHANEL_0,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANEL_1,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANEL_2,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANEL_3,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANEL_4,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANEL_5,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    }
};

static const struct iio_info iio_ad5592r_s_info = {
    .read_raw = &iio_ad5592r_s_read_raw,
    .write_raw = &iio_ad5592r_s_write_raw,
};


static int iio_ad5592r_s_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;
    
    indio_dev = devm_iio_device_alloc(&spi->dev,0);
    
    indio_dev->name ="iio_ad5592r_s";
    indio_dev->info = &iio_ad5592r_s_info;

    indio_dev->channels = iio_ad5592r_s_chanels;
    indio_dev->num_channels = 6;
    
    return devm_iio_device_register(&spi->dev,indio_dev);

}

struct spi_driver iio_ad5592r_s_driver = {
    .driver = {
        .name = "iio_ad5592r_s"
    },
    .probe = iio_ad5592r_s_probe
};
module_spi_driver(iio_ad5592r_s_driver);


MODULE_AUTHOR("Papp Richard <pappr805@gmail.com>");
MODULE_DESCRIPTION("ADC driver for COraz7s board");
MODULE_LICENSE("GPL v2");