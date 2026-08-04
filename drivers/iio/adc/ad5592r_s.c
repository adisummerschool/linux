// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * IIO-AD552R SPI ADC Driver
 *
 * Copyright 2011 Free Electrons
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

static const struct iio_info ad5592r_s_info = {

};

static int ad5592r_s_probe(struct spi_device *spi) {
    struct iio_dev *indio_dev;
    indio_dev = devm_iio_device_alloc(&spi->dev, 0);
    indio_dev->name = "ad5592r_s";
    indio_dev->info = &ad5592r_s_info;
    return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
    .driver = {
	.name = "ad5592r_s",
    },
    .probe = ad5592r_s_probe
};
module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Teodor Morosanu <teodor.ioan.morosanu@gmail.com>");
MODULE_DESCRIPTION("IIO-AD552R SPI ADC Driver");
MODULE_LICENSE("GPL");