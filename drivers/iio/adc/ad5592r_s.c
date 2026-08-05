// SPDX-License-Identifier: GPL-2.0-only
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2025 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>

 static int ad5592r_s_read_raw(struct iio_dev *indio_dev, //folosim pentru a citi datele de la driver
 				struct iio_chan_spec const *chan,
 				int *val, //returnam valarea prin referinta pentru a putea fi modificata in functie de ce citim
				int *val2, //folosim pentru floating point
				long mask)
 {
 	switch (mask) {
 	case IIO_CHAN_INFO_RAW:

	switch (chan->channel) {
		case 0:
			*val = 67;
			break;
		case 1:
			*val = 76;
			break;
		case 2:
			*val = 45;
			break;
		case 3:
			*val = 23;
			break;
		case 4:			
			*val = 12;
			break;
		case 5:
			*val = 34;
			break;
		default:
			return -EINVAL;
	}
		return IIO_VAL_INT;	

 	default:
 		return -EINVAL;
 	}
 }

  static int ad5592r_s_write_raw(struct iio_dev *indio_dev, //device(ar trebui sa fie canalul nostru), canalul, valoarea, valoarea2, masca
 				struct iio_chan_spec const *chan,
 				int val,
 				int val2,
 				long mask)
 {
		switch (mask) {
			case IIO_CHAN_INFO_RAW:
				switch (chan->channel) {
				case 0:
					dev_info(&indio_dev->dev, "Trying to write to channel 0\n");
					break;
				case 1:
					dev_info(&indio_dev->dev, "Trying to write to channel 1\n");
					break;
				case 2:
					dev_info(&indio_dev->dev, "Trying to write to channel 2\n");
					break;
				case 3:
					dev_info(&indio_dev->dev, "Trying to write to channel 3\n");
					break;
				case 4:
					dev_info(&indio_dev->dev, "Trying to write to channel 4\n");
					break;
				case 5:
					dev_info(&indio_dev->dev, "Trying to write to channel 5\n");
					break;
				default:
					return -EINVAL;
				}
				return 0;
			default:
				return -EINVAL;
			}
 }


  static const struct iio_chan_spec ad5592r_s_channels[] = { //confirurare canalelor, in cazul nostru 2 canale de tip tensiune
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 0,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 1,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
 	},
	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 2,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 3,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
 	},
	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 4,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 5,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
 	},
 };


 static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
 };


 static int ad5592r_s_probe(struct spi_device *spi)  //primeste ca parametru struct pentru ca e driver de spi
 {
 	struct iio_dev *indio_dev;

 	int ret;

 	indio_dev = devm_iio_device_alloc(&spi->dev, 0); //devm submodul linux, se ocupa de alocarea de memorie

    indio_dev->name = "ad5592r_s";
    indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = ad5592r_s_channels;
	indio_dev->num_channels = 6;

 	return devm_iio_device_register(&spi->dev, indio_dev);
 }

 struct spi_driver ad5592r_s_driver = {
 	.driver = {
 		.name = "ad5592r_s"
 	},
 	.probe = ad5592r_s_probe  //instantiaza driverul cu structura noastra
 };

 module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Pelin Mihai <mihai.pelin01@gmail.com>");
MODULE_DESCRIPTION("IIO-EMU SPI ADC driver");
MODULE_LICENSE("GPL v2");