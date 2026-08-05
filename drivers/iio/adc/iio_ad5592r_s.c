// SPDX-License-Identifier: GPL-2.0-only
/*
 * SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc (from AD5592 Driver)
 * Copyright 2012 CS Systemes d'Information
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>

 static const struct iio_info iio_ad5592r_info = {

 };

 static int iio_ad5592r_probe(struct spi_device *spi)
 {
    struct iio_dev *indio_dev;

    indio_dev = devm_iio_device_alloc(&spi->dev, 0);

    indio_dev->name = "iio_ad5592r_s";
    indio_dev->info = &iio_ad5592r_info;

    return devm_iio_device_register(&spi->dev, indio_dev);
 }

 static struct spi_driver iio_ad5592r_driver = {
    .driver = {
        .name = "iio_ad5592r_s"
    },
    .probe = iio_ad5592r_probe
 };

 module_spi_driver(iio_ad5592r_driver);

MODULE_AUTHOR("Doaga Petrut-Stefan <doaga.petrut.stefan@gmail.com");
MODULE_DESCRIPTION("Analog Devices IIO ADC EMU Summer School");
MODULE_LICENSE("GPL v2");