// SPDX-License-Identifier: GPL-2.0-only
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2025 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>


 static const struct iio_info iio_adc_emu_info = {

 };


 static int iio_adc_emu_probe(struct spi_device *spi)  //primeste ca parametru struct pentru ca e driver de spi
 {
 	struct iio_dev *indio_dev;

 	int ret;

 	indio_dev = devm_iio_device_alloc(&spi->dev, 0); //devm submodul linux, se ocupa de alocarea de memorie

    indio_dev->name = "iio_adc_emu";
    indio_dev->info = &iio_adc_emu_info;

 	return devm_iio_device_register(&spi->dev, indio_dev);
 }

 struct spi_driver iio_adc_emu_driver = {
 	.driver = {
 		.name = "iio_adc_emu"
 	},
 	.probe = iio_adc_emu_probe  //instantiaza driverul cu structura noastra
 };

 module_spi_driver(iio_adc_emu_driver);

MODULE_AUTHOR("Pelin Mihai <mihai.pelin01@gmail.com>");
MODULE_DESCRIPTION("IIO-EMU SPI ADC driver");
MODULE_LICENSE("GPL v2");