// SPDX-License-Identifier: GPL-2.0
/*
 * AD5592R SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2026 Papp Richard
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

const int CHANEL_0 = 0, CHANEL_1 = 1, CHANEL_2 = 2;
const int CHANEL_3 = 3, CHANEL_4 = 4, CHANEL_5 = 5;

struct iio_ad5592r_s_st {
	bool reg_select;
	int chan_val[6];
};

static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev,
				  struct iio_chan_spec const *chan, int *val,
				  int *val2, long mask)
{
	struct iio_ad5592r_s_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			switch (chan->channel) {
			case CHANEL_0:
				*val = st->chan_val[CHANEL_0];
				break;
			case CHANEL_1:
				*val = st->chan_val[CHANEL_1];
                                break;
			case CHANEL_2:
				*val = st->chan_val[CHANEL_2];
                                break;
			case CHANEL_3:
				*val = st->chan_val[CHANEL_3];
                                break;
			case CHANEL_4:
				*val = st->chan_val[CHANEL_4];
                                break;
			case CHANEL_5:
				*val = st->chan_val[CHANEL_5];
                                break;
			default:
				return -EINVAL;
			}
			return IIO_VAL_INT;
		} else
			return -EINVAL;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int iio_ad5592r_s_write_raw(struct iio_dev *indio_dev,
				   struct iio_chan_spec const *chan, int val,
				   int val2, long mask)
{
	struct iio_ad5592r_s_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			switch (chan->channel) {
			case CHANEL_0:
				dev_info(&indio_dev->dev, "Trying to write to chanel 0");
				st->chan_val[CHANEL_0] = val;
				return 0;
			case CHANEL_1:
				dev_info(&indio_dev->dev,"Trying to write to chanel 1");
				st->chan_val[CHANEL_1] = val;
				return 0;
			case CHANEL_2:
				dev_info(&indio_dev->dev, "Trying to write to chanel 2");
				st->chan_val[CHANEL_2] = val;
				return 0;
			case CHANEL_3:
				dev_info(&indio_dev->dev, "Trying to write to chanel 3");
				st->chan_val[CHANEL_3] = val;
				return 0;
			case CHANEL_4:
				dev_info(&indio_dev->dev, "Trying to write to chanel 4");
				st->chan_val[CHANEL_4] = val;
				return 0;
			case CHANEL_5:
				dev_info(&indio_dev->dev, "Trying to write to chanel 5");
				st->chan_val[CHANEL_5] = val;
				return 0;
			default:
				return -EINVAL;
			}
		} else
			return -EINVAL;
	case IIO_CHAN_INFO_ENABLE:
		st->reg_select = val ? 1 : 0;
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct iio_chan_spec iio_ad5592r_s_chanels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = CHANEL_0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = CHANEL_1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = CHANEL_2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = CHANEL_3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = CHANEL_4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = CHANEL_5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	}
};

static const struct iio_info iio_ad5592r_s_info = {
	.read_raw = &iio_ad5592r_s_read_raw,
	.write_raw = &iio_ad5592r_s_write_raw,
};

static int iio_ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_ad5592r_s_st *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	indio_dev->name = "iio_ad5592r_s";
	indio_dev->info = &iio_ad5592r_s_info;

	indio_dev->channels = iio_ad5592r_s_chanels;
	indio_dev->num_channels = ARRAY_SIZE(iio_ad5592r_s_chanels);

	st = iio_priv(indio_dev);
	st->reg_select = 1;
	memset(st->chan_val, 0, sizeof(st->chan_val));

	return devm_iio_device_register(&spi->dev, indio_dev);
}

struct spi_driver iio_ad5592r_s_driver = { .driver = { .name = "iio_ad5592r_s" },
					   .probe = iio_ad5592r_s_probe };
module_spi_driver(iio_ad5592r_s_driver);

MODULE_AUTHOR("Papp Richard <pappr805@gmail.com>");
MODULE_DESCRIPTION("ADC driver for COraz7s board");
MODULE_LICENSE("GPL v2");