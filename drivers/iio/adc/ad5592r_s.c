// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * IIO-AD552R SPI ADC Driver
 *
 * Copyright 2011 Free Electrons
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

static int adc_ad5992r_s_read_raw(struct iio_dev* indio_dev, struct iio_chan_spec const* chan, int* val, int* val2, long mask) {
	switch (mask) {
		case IIO_CHAN_INFO_RAW:
			switch(chan->channel) {
				case 0: *val = 0; break;
				case 1: *val = 1; break;
				case 2: *val = 2; break;
				case 3: *val = 3; break;
				case 4: *val = 4; break;
				case 5: *val = 5; break;
				default: *val = -1; break;
			}
			return IIO_VAL_INT;
		default:
			return -EINVAL;
	}
}

static int adc_ad5992r_s_write_raw(struct iio_dev* indio_dev, struct iio_chan_spec const* chan, int val, int val2, long mask) {
	switch (mask) {
		case IIO_CHAN_INFO_RAW:
			switch(chan->channel) {
				case 0: dev_info(&indio_dev->dev, "Trying to write first channel (0)"); break;
				case 1: dev_info(&indio_dev->dev, "Trying to write second channel (1)"); break;
				case 2: dev_info(&indio_dev->dev, "Trying to write third channel (2)"); break;
				case 3: dev_info(&indio_dev->dev, "Trying to write fourth channel (3)"); break;
				case 4: dev_info(&indio_dev->dev, "Trying to write fifth channel (4)"); break;
				case 5: dev_info(&indio_dev->dev, "Trying to write sixth channel (5)"); break;
				default: dev_info(&indio_dev->dev, "Trying to write non-valid channel (-1)"); break;
			}
			return 0;
		default:
			return -EINVAL;
	}
}

static const struct iio_chan_spec adc_ad5992r_s_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
   }
};

static const struct iio_info ad5592r_s_info = {
	.read_raw = &adc_ad5992r_s_read_raw,
	.write_raw = &adc_ad5992r_s_write_raw,
};

static int ad5592r_s_probe(struct spi_device *spi) {
    struct iio_dev *indio_dev;
    indio_dev = devm_iio_device_alloc(&spi->dev, 0);
    indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = adc_ad5992r_s_channels;
	indio_dev->num_channels = 6;
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