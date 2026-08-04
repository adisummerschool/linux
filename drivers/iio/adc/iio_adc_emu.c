// SPDX-License Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

static const struct iio_info iio_adc_emu_info = {

};

static int iio_adc_emu_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;

    //pentru alocarea memoriei
    indio_dev = devm_iio_device_alloc(&spi->dev, 0);

    //popularea structurii
    indio_dev->name = "iio_adc_emu";
    indio_dev->info = &iio_adc_emu_info;

    return devm_iio_device_register(&spi->dev,indio_dev);
}
static struct spi_driver iio_adc_emu_driver = {
    .driver = {
            .name = "iio_adc_emu"
    },
    .probe = iio_adc_emu_probe
};
module_spi_driver(iio_adc_emu_driver);

MODULE_AUTHOR("Tomescu Lucas <lucastomescu@gmail.com>");
MODULE_DESCRIPTION("Analog Devices IIO ADC EMU Summer School");
MODULE_LICENSE("GPL v2");

