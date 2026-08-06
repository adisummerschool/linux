#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

struct iio_ad5592r_s_st {
	int reg_select;
	int chan_val[6];
};

static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev,
				  struct iio_chan_spec const *chan, int *val,
				  int *val2, long mask)
{
	struct iio_ad5592r_s_st *st = iio_priv(indio_dev);

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
		} else {
			return -EINVAL;
		}
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
		if(!st->reg_select) {
			switch (chan->channel) {
			case 0:
				st->chan_val[0] = val;
				dev_info(&indio_dev->dev,
					"Writing to channel 0");
				break;
			case 1:
				st->chan_val[1] = val;
				dev_info(&indio_dev->dev,
					"Writing to channel 1");
				break;
			case 2:
				st->chan_val[2] = val;
				dev_info(&indio_dev->dev,
					"Writing to channel 2");
				break;
			case 3:
				st->chan_val[3] = val;
				dev_info(&indio_dev->dev,
					"Writing to channel 3");
				break;
			case 4:
				st->chan_val[4] = val;
				dev_info(&indio_dev->dev,
					"Writing to channel 4");
				break;
			case 5:
				st->chan_val[5] = val;
				dev_info(&indio_dev->dev,
					"Writing to channel 5");
				break;
			default:
				return -EINVAL;
			}
			return 0;
		} else {
			return -EINVAL;
		}
	case IIO_CHAN_INFO_ENABLE:
		st->reg_select = val ? 1 : 0;
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct iio_chan_spec iio_ad5592r_s_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
	}
};

static const struct iio_info ad5592r_s_info = {
	.read_raw = &iio_ad5592r_s_read_raw,
	.write_raw = &iio_ad5592r_s_write_raw
};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_ad5592r_s_st *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->reg_select = 1; //inverse logic, 1 = powered off
	memset(st->chan_val, 0, sizeof(st->chan_val));
	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = iio_ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(iio_ad5592r_s_channels);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = { .driver = { .name = "ad5592r_s" },
					      .probe = ad5592r_s_probe };

module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Berinde Codrin <codrin.berinde@gmail.com>");
MODULE_DESCRIPTION("IIO AD5592R_S");
MODULE_LICENSE("GPL v2");