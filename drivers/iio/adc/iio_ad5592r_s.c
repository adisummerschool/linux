// SPDX-License-Identifier: GPL-2.0-only
/*
 * AD7785/AD7792/AD7793/AD7794/AD7795 SPI ADC driver
 *
 * Copyright 2011-2012 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>

 static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev,
                               struct iio_chan_spec const *chan,
                               int *val,
                               int *val2,
                               long mask)
  {
   switch (mask){
      case IIO_CHAN_INFO_RAW:
              switch(chan->channel){
               case 0: *val = 0;
               return IIO_VAL_INT;
               case 1: *val = 1;
               return IIO_VAL_INT;
               case 2: *val = 2;
               return IIO_VAL_INT;
               case 3: *val = 3;
               return IIO_VAL_INT;
               case 4: *val = 4;
               return IIO_VAL_INT;
               case 5: *val = 5;
               return IIO_VAL_INT;
              }
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
   switch (mask){
      case IIO_CHAN_INFO_RAW:
              switch(chan->channel){
               case 0: dev_info(&indio_dev->dev, "Trying to write to channel 0");
               return 0;
               case 1: dev_info(&indio_dev->dev, "Trying to write to channel 1");
               return 0;
               case 2: dev_info(&indio_dev->dev, "Trying to write to channel 2");
               return 0;
               case 3: dev_info(&indio_dev->dev, "Trying to write to channel 3");
               return 0;
               case 4: dev_info(&indio_dev->dev, "Trying to write to channel 4");
               return 0;
               case 5: dev_info(&indio_dev->dev, "Trying to write to channel 5");
               return 0;
              }      
      default:
             return -EINVAL;
   }
  }                               
                              

 static const struct iio_chan_spec iio_ad5592r_s_channels[] = {
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
 static const struct iio_info iio_ad5592r_s_info = {
        .read_raw = &iio_ad5592r_s_read_raw,
        .write_raw = &iio_ad5592r_s_write_raw

 };
 

 static int iio_ad5592r_s_probe(struct spi_device *spi)
 {
    struct iio_dev *indio_dev;

    indio_dev = devm_iio_device_alloc(&spi->dev, 0);

    indio_dev->name = "iio_ad5592r_s";
    indio_dev->info = &iio_ad5592r_s_info;
    indio_dev->channels = iio_ad5592r_s_channels;
    indio_dev->num_channels = 6;

    return devm_iio_device_register(&spi->dev, indio_dev);
 }

 static struct spi_driver iio_ad5592r_s_driver = {
    .driver = {
        .name = "iio_ad5592r_s"
    },
    .probe = iio_ad5592r_s_probe
 };
 module_spi_driver(iio_ad5592r_s_driver);


MODULE_AUTHOR("Cucu Maria-virginia <mariavirginia.cucu@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD5592");
MODULE_LICENSE("GPL v2");