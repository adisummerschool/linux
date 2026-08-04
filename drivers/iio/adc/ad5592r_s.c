// SPDX-License-Identifier: GPL-2.0
/*
 * IIO SPI ADC driver
 *
 * Copyright 2026 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

struct iio_adc_st {
	bool reg_select;
	int chan_val[6];
};

static const struct iio_chan_spec iio_adc_channels[] = {
	{ .type = IIO_VOLTAGE,
	  .channel = 0,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 1,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 2,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 3,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)

	},
	{ .type = IIO_VOLTAGE,
	  .channel = 4,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 5,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
};

static int iio_adc_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan, int *val,
			    int *val2, long mask)
{
	struct iio_adc_st *st = iio_priv(indio_dev);
	// citim datele
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		// citim datele de la ADC
		if (!st->reg_select) {
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
			}
		} else {
			return -EINVAL;
		}
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select ? 1 : 0;
		return 0;

	default:
		return -EINVAL;
	}
	return 0;
}

static int iio_adc_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan, int val,
			     int val2, long mask)
{
	struct iio_adc_st *st = iio_priv(indio_dev);
	// scriem datele
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			switch (chan->channel) {
			case 0:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 0");
				st->chan_val[0] = val;
				break;
			case 1:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 1");
				st->chan_val[1] = val;
				break;
			case 2:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 2");
				st->chan_val[2] = val;
				break;
			case 3:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 3");
				st->chan_val[3] = val;
				break;
			case 4:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 4");
				st->chan_val[4] = val;
				break;
			case 5:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 5");
				st->chan_val[5] = val;
				break;
			}
		} else {
			return -EINVAL;
		}
		return 0;
	case IIO_CHAN_INFO_ENABLE:
		st->reg_select = val ? true : false;
		return 0;
	default:
		return -EINVAL;
	}
	return 0;
	//returnam datele scrise
}

static const struct iio_info ad5592r_info = { .read_raw = &iio_adc_read_raw,
					      .write_raw = &iio_adc_write_raw };
static int ad5592r_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_adc_st *st;

	//pentru alocarea memoriei
	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->reg_select = true;
	memset(st->chan_val, 0, sizeof(st->chan_val));
	//popularea structurii
	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_info;
	indio_dev->channels = iio_adc_channels;
	indio_dev->num_channels = 6;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_driver = { .driver = { .name = "ad5592r_s" },
					    .probe = ad5592r_probe };

module_spi_driver(ad5592r_driver);
MODULE_AUTHOR("Tomescu Lucas <lucastomescu@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD5592R_S IIO SPI ADC driver for Summer School");
MODULE_LICENSE("GPL");
