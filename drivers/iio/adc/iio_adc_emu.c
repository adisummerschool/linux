// SPDX-License-Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>


static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,struct iio_chan_spec const *chan,int *val,int *val2,long mask)
    {
        switch (mask) 
            {
                case IIO_CHAN_INFO_RAW:
                    if(chan->channel)
                        *val = 67;
                    else
                        *val=76;
                    return  IIO_VAL_INT;   
                default:
                    return -EINVAL;
            }
    }
 
static int iio_adc_emu_write_raw(struct iio_dev *indio_dev,struct iio_chan_spec const *chan, int val, int val2, long mask)
    {
        switch(mask)
        {
            case IIO_CHAN_INFO_RAW:
                if(chan->channel)
                    dev_info(&indio_dev->dev,"Trying to write to channel 1");
                else 
                    dev_info(&indio_dev->dev,"Trying to write to channel 2");   
                return 0;    
            default:
                return -EINVAL;     
        }    
    }

  static const struct iio_chan_spec iio_adc_emu_channels[]={
    {
        .type =IIO_VOLTAGE,
        .channel= 0,
        .indexed= 1,
        .info_mask_separate=BIT(IIO_CHAN_INFO_RAW),
    },

    {
        .type =IIO_VOLTAGE,
        .channel= 1,
        .indexed= 1,
        .info_mask_separate=BIT(IIO_CHAN_INFO_RAW),
    },

  };

  static const struct iio_info iio_adc_emu_info ={
        .read_raw= &iio_adc_emu_read_raw,
        .write_raw= &iio_adc_emu_write_raw,
  };


//in linux se face prin static ca in acest fisier sa fie chemat exact functia asta insine
 static int iio_adc_probe(struct spi_device *spi)
    {
        struct iio_dev *indio_dev;
        
        indio_dev = devm_iio_device_alloc(&spi->dev,0);

        indio_dev->name="iio_adc_emu";
        indio_dev->info=&iio_adc_emu_info;
        indio_dev->channels=iio_adc_emu_channels;
        indio_dev->num_channels=2;

        

        return devm_iio_device_register(&spi->dev,indio_dev);
        
    }

 static struct spi_driver iio_adc_emu_driver={
        .driver={
            .name="iio_driver"
        },
        .probe = iio_adc_probe
    };


module_spi_driver(iio_adc_emu_driver);

    MODULE_AUTHOR("Molnar Levente <mlevente388@gmail.com>");
    MODULE_DESCRIPTION("IIO setup for and ADC5592-summer practice setup");
    MODULE_LICENSE("GPL v2");