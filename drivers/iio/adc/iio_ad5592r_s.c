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

static const struct iio_info iio_ad5592r_s_info = {

};


static int iio_ad5592r_s_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;
    
    indio_dev = devm_iio_device_alloc(&spi->dev,0);
    
    indio_dev->name ="iio_ad5592r_s";
    indio_dev->info = &iio_ad5592r_s_info;
    
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