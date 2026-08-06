// SPDX-License-Identifier: GPL-2.0-only
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2025 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>

 struct ad5592r_s_st{
 	int reg_select;
	
	int chan_val[6];
 };

 static int ad5592r_s_read_raw(struct iio_dev *indio_dev, //folosim pentru a citi datele de la driver
 				struct iio_chan_spec const *chan,
 				int *val, //returnam valarea prin referinta pentru a putea fi modificata in functie de ce citim
				int *val2, //folosim pentru floating point
				long mask)
 {
	struct ad5592r_s_st *st = iio_priv(indio_dev); //folosim pentru a accesa structura noastra

 	switch (mask) {
 	case IIO_CHAN_INFO_RAW:
	if(!st->reg_select) {
		switch (chan->channel) {
			case 0:
				*val = st->chan_val[0];
				break;
			case 1:
				*val = st->chan_val[1];
				break;
			case 2:
				*val = st->chan_val[2];
				break;
			case 3:
				*val = st->chan_val[3];
				break;
			case 4:			
				*val = st->chan_val[4];
				break;
			case 5:
				*val = st->chan_val[5];
				break;
			default:
				return -EINVAL;
		}
			return IIO_VAL_INT;	
	}
	else
		return -EINVAL;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select;
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
	struct ad5592r_s_st *st = iio_priv(indio_dev); //folosim pentru a accesa structura noastra

		switch (mask) {
			case IIO_CHAN_INFO_RAW:
			if(!st->reg_select) {
				switch (chan->channel) {
				case 0:
					dev_info(&indio_dev->dev, "Trying to write to channel 0\n");
					st->chan_val[0] = val;
					break;
				case 1:
					dev_info(&indio_dev->dev, "Trying to write to channel 1\n");
					st->chan_val[1] = val;
					break;
				case 2:
					dev_info(&indio_dev->dev, "Trying to write to channel 2\n");
					st->chan_val[2] = val;
					break;
				case 3:
					dev_info(&indio_dev->dev, "Trying to write to channel 3\n");
					st->chan_val[3] = val;
					break;
				case 4:
					dev_info(&indio_dev->dev, "Trying to write to channel 4\n");
					st->chan_val[4] = val;
					break;
				case 5:
					dev_info(&indio_dev->dev, "Trying to write to channel 5\n");
					st->chan_val[5] = val;
					break;
				default:
					return -EINVAL;
				}
				return 0;
			}
			else
				return -EINVAL;
			case IIO_CHAN_INFO_ENABLE:
				st->reg_select = val ? 1 : 0;
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
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 1,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 2,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 3,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 4,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 5,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 };


 static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
 };


 static int ad5592r_s_probe(struct spi_device *spi)  //primeste ca parametru struct pentru ca e driver de spi
 {
 	struct iio_dev *indio_dev;
	struct ad5592r_s_st *st;

 	int ret;

 	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st)); //devm submodul linux, se ocupa de alocarea de memorie
	st = iio_priv(indio_dev); //returneaza pointerul catre structura noastra
	st->reg_select = 1;
	memset(st->chan_val, 0, sizeof(st->chan_val)); //initializam valorile canalelor cu 0
    indio_dev->name = "ad5592r_s";
    indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channels);

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