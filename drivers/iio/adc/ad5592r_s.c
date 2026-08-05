// SPDX-License-Identifier: GPL-2.0
/*
 * AD5592R SPI ADC driver
 *
 * Copyright 2011-2015 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

static int ad5592r_s_read_raw(struct iio_dev *indio_dev,
			      const struct iio_chan_spec *chan,
			      int *val,
			      int *val2,
			      long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		*val = chan->channel * 100;

		return IIO_VAL_INT;

	default:
		return -EINVAL;
	}
}

static int ad5592r_s_write_raw(struct iio_dev *indio_dev,
			       const struct iio_chan_spec *chan,
			       int val,
			       int val2,
			       long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		dev_info(&indio_dev->dev,
			 "Try to write value %d to channel %d\n",
			 val, chan->channel);

		return 0;

	default:
		return -EINVAL;
	}
}

#define AD5592R_S_CHANNEL(_channel)			\
	{						\
		.type = IIO_VOLTAGE,			\
		.indexed = 1,				\
		.channel = (_channel),			\
		.info_mask_separate =			\
			BIT(IIO_CHAN_INFO_RAW),		\
	}

static const struct iio_chan_spec ad5592r_s_channels[] = {
	AD5592R_S_CHANNEL(0),
	AD5592R_S_CHANNEL(1),
	AD5592R_S_CHANNEL(2),
	AD5592R_S_CHANNEL(3),
	AD5592R_S_CHANNEL(4),
	AD5592R_S_CHANNEL(5),
};

static const struct iio_info ad5592r_s_info = {
	.read_raw = ad5592r_s_read_raw,
	.write_raw = ad5592r_s_write_raw,
};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if (!indio_dev)
		return -ENOMEM;

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channels);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
	.driver = {
		.name = "ad5592r_s",
	},
	.probe = ad5592r_s_probe,
};

module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Strava Cosmin-Paul <kosmin.strava@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD5592R IIO SPI ADC driver");
MODULE_LICENSE("GPL v2");