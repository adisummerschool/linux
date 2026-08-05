// SPDX-License-Identifier: GPL-2.0
/*
 * IIO CORA SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>

 static int iio_adc_cora_read_raw(struct iio_dev *indio_dev, 
                              struct iio_chan_spec const *chan, 
                              int *val, 
                              int *val2, 
                              long mask)
{
    switch(mask) {
      case IIO_CHAN_INFO_RAW:
            if (chan->channel == 0)
               *val = 67;
            else if (chan->channel == 1)
               *val = 76;
            else if (chan->channel == 2)
               *val = 85;
            else if (chan->channel == 3)
               *val = 94;
            else if (chan->channel == 4)
               *val = 103;
            else if (chan->channel == 5)
               *val = 112;
            return IIO_VAL_INT;

      default: 
            return -EINVAL;
    }
}

 static int iio_adc_cora_write_raw(struct iio_dev *indio_dev, 
                              struct iio_chan_spec const *chan, 
                              int val, 
                              int val2, 
                              long mask)      
{
   switch(mask) {
      case IIO_CHAN_INFO_RAW:
            if (chan->channel == 0)
               dev_info(&indio_dev->dev, "Trying to write to channel 0: %d\n", val);
            else if (chan->channel == 1)
               dev_info(&indio_dev->dev, "Trying to write to channel 1: %d\n", val);
            else if (chan->channel == 2)
               dev_info(&indio_dev->dev, "Trying to write to channel 2: %d\n", val);
            else if (chan->channel == 3)
               dev_info(&indio_dev->dev, "Trying to write to channel 3: %d\n", val);
            else if (chan->channel == 4)
               dev_info(&indio_dev->dev, "Trying to write to channel 4: %d\n", val);
            else if (chan->channel == 5)
               dev_info(&indio_dev->dev, "Trying to write to channel 5: %d\n", val);
            return 0;

      default:
         return -EINVAL; 
   }
}

 static const struct iio_chan_spec iio_adc_cora_channels[] = {
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
   },
   {
      .type = IIO_VOLTAGE,
      .channel = 2,
      .indexed = 1,
      .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
   },
   {
      .type = IIO_VOLTAGE,
      .channel = 3,
      .indexed = 1,
      .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
   },
   {
      .type = IIO_VOLTAGE,
      .channel = 4,
      .indexed = 1,
      .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
   },
   {
      .type = IIO_VOLTAGE,
      .channel = 5,
      .indexed = 1,
      .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
   }
 };

  static const struct iio_info iio_adc_cora_info = {
   .read_raw = &iio_adc_cora_read_raw,
   .write_raw = &iio_adc_cora_write_raw
 };

 static int iio_adc_probe(struct spi_device *spi){

    struct iio_dev *indio_dev;
    
    indio_dev = devm_iio_device_alloc(&spi->dev, 0);

    indio_dev->name = "ad5592r_s";
    indio_dev->info = &iio_adc_cora_info;
    indio_dev->channels = iio_adc_cora_channels;
    indio_dev->num_channels = 6;

    return devm_iio_device_register(&spi->dev, indio_dev);
 }

 static struct spi_driver iio_adc_driver = {
    .driver = {
        .name = "ad5592r_s"
    },
    .probe = iio_adc_probe
 };
 module_spi_driver(iio_adc_driver);
 
 MODULE_AUTHOR("Moldovan Flavius <flaviuss3035@gmail.com>");
 MODULE_DESCRIPTION("Analog Devices IIO CORA Summer School");
 MODULE_LICENSE("GPL v2");