// SPDX-Licennse-Identifier: GPL-2.0
/*
 * IIO-ad5592r_s SPI ADC Driver
 *
 * Copyright 2013-2015 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>

 static const struct iio_info ad5592r_s_info = {

 };

 static int ad5592r_s_probe(struct spi_device *spi)
 {
    struct iio_dev *indio_dev;

    indio_dev = devm_iio_device_alloc(&spi->dev, 0);

    indio_dev->name = "ad5592r_s";
    indio_dev->info = &ad5592r_s_info;

    return devm_iio_device_register(&spi->dev, indio_dev);
 }

 static struct spi_driver ad5592r_s_driver = {
    .driver = {
        .name = "ad5592r_s"
    },
    .probe = ad5592r_s_probe
 };
 module_spi_driver(ad5592r_s_driver);

 MODULE_AUTHOR("Dradici Leon <dradici.leon5@gmail.com>");
 MODULE_DESCRIPTION("Analog Devices AD5592R_S SPI ADC Driver");
 MODULE_LICENSE("GPL v2");