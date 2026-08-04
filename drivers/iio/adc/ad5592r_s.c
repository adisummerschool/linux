// SPDX-License Identifier: GPL-2.0 /*
//  * IIO-EMU SPI ADC driver *
//  * Copyright 2026 Analog Devices Inc. *
// 
#include <linux/module.h> 
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
static const struct iio_info ad5592r_info = { };
static int ad5592r_probe(struct spi_device *spi){ 
struct iio_dev *indio_dev; 

//pentru alocarea memoriei 
indio_dev = devm_iio_device_alloc(&spi->dev, 0);

//popularea structurii 
indio_dev->name = "ad5592r_s";
indio_dev->info = &ad5592r_info; 
return devm_iio_device_register(&spi->dev,indio_dev);
} 
static struct spi_driver ad5592r_driver = {
         .driver = { .name = "ad5592r_s" }, 
         .probe = ad5592r_probe 
};

         module_spi_driver(ad5592r_driver);
MODULE_AUTHOR("Tomescu Lucas <lucastomescu@gmail.com>");
MODULE_DESCRIPTION("Analog Devices  AD5592R_S EMU Summer School"); 
MODULE_LICENSE("GPL v2");
