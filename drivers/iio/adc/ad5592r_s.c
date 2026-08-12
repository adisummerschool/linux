// SPDX-License-Identifier: GPL-2.0-only
/*
 * AD5592R simplified SPI ADC driver
 *
 * Copyright 2025 Analog Devices Inc.
 */

#include <linux/unaligned.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

#define AD5592R_S_MSB_MSK			BIT(15)
#define AD5592R_S_ADDR_MSK			GENMASK(14, 11)
#define AD5592R_S_DATA_MSK			GENMASK(10, 0)

#define AD5592R_S_REG_RDB_ADDR			0x7
#define AD5592R_S_EN_READB			BIT(6)
#define AD5592R_S_REG_SELECT_RDB		GENMASK(5, 2)

#define AD5592R_S_REG_PD_ADDR			0xB
#define AD5592R_S_REG_EN_IREF			BIT(9)

#define AD5592R_S_REG_ADC_SEQ			0x2
#define AD5592R_S_REG_ADC_CONFIG		0x4

#define AD5592R_S_ADC_CH_MSK			GENMASK(14, 12)
#define AD5592R_S_ADC_DATA_MSK			GENMASK(11, 0)

struct ad5592r_s_st {
	int reg_select;
	int chan_val[6];
	struct spi_device *spi;
};

static int ad5592r_s_spi_write(struct ad5592r_s_st *st,
			       u8 addr, u16 data)
{
	u16 tx = 0;
	u16 package = 0;

	struct spi_transfer t = {
		.tx_buf = &package,
		.len = 2,
	};

	tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) |
	     FIELD_PREP(AD5592R_S_ADDR_MSK, addr) |
	     FIELD_PREP(AD5592R_S_DATA_MSK, data);

	put_unaligned_be16(tx, &package);

	dev_info(&st->spi->dev, "tx we constructed: %x\n", tx);
	dev_info(&st->spi->dev, "package we constructed: %x\n", package);

	return spi_sync_transfer(st->spi, &t, 1);
}

static int ad5592r_s_spi_read(struct ad5592r_s_st *st,
			      u8 addr, u16 *data)
{
	u16 rx = 0;
	u16 reg_rdb_data;
	u16 rcv_data = 0;
	int ret;

	struct spi_transfer t = {
		.rx_buf = &rx,
		.len = 2,
	};

	reg_rdb_data =
		FIELD_PREP(AD5592R_S_EN_READB, 1) |
		FIELD_PREP(AD5592R_S_REG_SELECT_RDB, addr);

	ret = ad5592r_s_spi_write(st,
				  AD5592R_S_REG_RDB_ADDR,
				  reg_rdb_data);
	if (ret) {
		dev_err(&st->spi->dev,
			"Writing the readback register failed\n");
		return ret;
	}

	ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret) {
		dev_err(&st->spi->dev,
			"Failed receiving readback\n");
		return ret;
	}

	rcv_data = get_unaligned_be16(&rx);

	*data = FIELD_GET(AD5592R_S_DATA_MSK, rcv_data);

	return 0;
}

static int ad5592r_s_spi_transfer(struct ad5592r_s_st *st,
				  u16 *data)
{
	u16 tx = 0;
	u16 rx = 0;
	int ret;

	struct spi_transfer t = {
		.tx_buf = &tx,
		.rx_buf = &rx,
		.len = 2,
	};

	ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret) {
		dev_err(&st->spi->dev,
			"SPI transfer failed\n");
		return ret;
	}

	*data = get_unaligned_be16(&rx);

	return 0;
}

static int ad5592r_s_read_chan(struct ad5592r_s_st *st,
			       int channel, u16 *readval)
{
	u16 data = 0;
	int ret;

	ret = ad5592r_s_spi_write(st,
				  AD5592R_S_REG_ADC_SEQ,
				  BIT(channel));
	if (ret) {
		dev_err(&st->spi->dev,
			"Writing ADC sequence failed: %d\n",
			ret);
		return ret;
	}

	ndelay(500);

	/*
	 * First transfer starts the ADC conversion.
	 */
	ret = ad5592r_s_spi_transfer(st, &data);
	if (ret)
		return ret;

	udelay(2);

	/*
	 * Second transfer receives the ADC result.
	 */
	ret = ad5592r_s_spi_transfer(st, &data);
	if (ret)
		return ret;

	dev_info(&st->spi->dev,
		 "ADC channel %d frame: 0x%04x\n",
		 channel, data);

	if (FIELD_GET(AD5592R_S_ADC_CH_MSK, data) != channel) {
		dev_err(&st->spi->dev,
			"Unexpected ADC channel\n");
		return -EIO;
	}

	*readval = FIELD_GET(AD5592R_S_ADC_DATA_MSK, data);

	return 0;
}

static int ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev,
					unsigned int reg,
					unsigned int writeval,
					unsigned int *readval)
{
	struct ad5592r_s_st *st = iio_priv(indio_dev);
	u16 data;
	int ret;

	if (!readval)
		return ad5592r_s_spi_write(st, reg, writeval);

	ret = ad5592r_s_spi_read(st, reg, &data);
	if (ret)
		return ret;

	*readval = data;

	return 0;
}

static int ad5592r_s_read_raw(struct iio_dev *indio_dev,
			      struct iio_chan_spec const *chan,
			      int *val,
			      int *val2,
			      long mask)
{
	struct ad5592r_s_st *st = iio_priv(indio_dev);
	u16 data;
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (st->reg_select)
			return -EINVAL;

		ret = ad5592r_s_read_chan(st, chan->channel, &data);
		if (ret) {
			dev_err(&indio_dev->dev,
				"Reading channel %d failed: %d\n",
				chan->channel, ret);
			return ret;
		}

		*val = data;

		return IIO_VAL_INT;

	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select;

		return IIO_VAL_INT;

	default:
		return -EINVAL;
	}
}

static int ad5592r_s_write_raw(struct iio_dev *indio_dev,
			       struct iio_chan_spec const *chan,
			       int val,
			       int val2,
			       long mask)
{
	struct ad5592r_s_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			switch (chan->channel) {
			case 0:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 0\n");
				st->chan_val[0] = val;
				break;

			case 1:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 1\n");
				st->chan_val[1] = val;
				break;

			case 2:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 2\n");
				st->chan_val[2] = val;
				break;

			case 3:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 3\n");
				st->chan_val[3] = val;
				break;

			case 4:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 4\n");
				st->chan_val[4] = val;
				break;

			case 5:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 5\n");
				st->chan_val[5] = val;
				break;

			default:
				return -EINVAL;
			}

			return 0;
		}

		return -EINVAL;

	case IIO_CHAN_INFO_ENABLE:
		st->reg_select = val ? 1 : 0;

		return 0;

	default:
		return -EINVAL;
	}
}

#define AD5592R_S_CHANNEL(_channel)				\
{								\
	.type = IIO_VOLTAGE,					\
	.indexed = 1,						\
	.channel = (_channel),					\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		\
	.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),	\
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
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
	.debugfs_reg_access = &ad5592r_s_debugfs_reg_access,
};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ad5592r_s_st *st;
	int ret;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);

	st->reg_select = 1;
	st->spi = spi;

	memset(st->chan_val, 0, sizeof(st->chan_val));

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	/*
	 * Enable the internal reference.
	 */
	ret = ad5592r_s_spi_write(st,
				  AD5592R_S_REG_PD_ADDR,
				  FIELD_PREP(AD5592R_S_REG_EN_IREF, 1));
	if (ret)
		return ret;

	/*
	 * Configure I/O0 - I/O5 as ADC inputs.
	 * This is temporary for the current implementation.
	 */
	ret = ad5592r_s_spi_write(st,
				  AD5592R_S_REG_ADC_CONFIG,
				  GENMASK(5, 0));
	if (ret)
		return ret;

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
MODULE_DESCRIPTION("Analog Devices AD5592R simplified IIO SPI ADC driver");
MODULE_LICENSE("GPL");