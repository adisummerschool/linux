// SPDX-License-Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>


  static const struct iio_info adc_ad5592_driver_info ={};


//in linux se face prin static ca in acest fisier sa fie chemat exact functia asta insine
 static int ad5592r_s_probe(struct spi_device *spi)
    {
        struct iio_dev *indio_dev;
        
        indio_dev = devm_iio_device_alloc(&spi->dev,0);

        indio_dev->name="ad5592r_s";
        indio_dev->info=&adc_ad5592_driver_info;
        return devm_iio_device_register(&spi->dev,indio_dev);
        
    }

 static struct spi_driver ad5592r_s_driver={
        .driver={
            .name="ad5592r_s"
        },
        .probe = ad5592r_s_probe
    };


module_spi_driver(ad5592r_s_driver);

    MODULE_AUTHOR("Molnar Levente <mlevente388@gmail.com>");
    MODULE_DESCRIPTION("IIO setup for and AD5592-summer practice setup");
    MODULE_LICENSE("GPL v2");