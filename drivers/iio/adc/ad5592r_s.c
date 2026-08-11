// SPDX-License-Identifier: GPL-2.0
/*
 * AD5592R SPI ADC driver
 *
 * Copyright 2011-2015 Analog Devices Inc.
 */

#include <linux/unaligned.h>
#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

#define AD5592R_S_NUM_CHANNELS	6

#define AD5592R_S_MSB_MSK BIT(15)
#define AD5592R_ADDR_MSK GENMASK(14, 11)
#define AD5592R_DATA_MSK GENMASK(10, 0)

#define AD5592R_EN_READB BIT(6)
#define AD5592R_REG_RDB_ADDR 0x7
#define AD5592R_S_REG_SELECT_RDB GENMASK(5, 2)
#define AD5592R_REG_PD_ADDR 0xB
#define AD5592R_REG_EN_IREF BIT(9)

struct ad5592r_s_state {
	int reg_select;
	int chan_val[AD5592R_S_NUM_CHANNELS];
	struct spi_device *spi;
};

static int ad5592r_s_read_raw(struct iio_dev *indio_dev,
			      const struct iio_chan_spec *chan,
			      int *val,
			      int *val2,
			      long mask)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (st->reg_select)
			return -EINVAL;

		*val = st->chan_val[chan->channel];

		return IIO_VAL_INT;

	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select;

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
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (st->reg_select)
			return -EINVAL;

		st->chan_val[chan->channel] = val;

		dev_info(&indio_dev->dev,
			 "Write value %d to channel %d\n",
			 val, chan->channel);

		return 0;

	case IIO_CHAN_INFO_ENABLE:
		st->reg_select = val ? 1 : 0;

		return 0;

	default:
		return -EINVAL;
	}
}

static int ad5592r_s_spi_write(struct ad5592r_s_state *st,
			       u8 addr, u16 data)
{
	u16 tx = 0;
	u16 packed = 0;

	struct spi_transfer t = {
		.tx_buf = &packed,
		.len = 2,
	};

	tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) | FIELD_PREP(AD5592R_ADDR_MSK, addr) |
	     FIELD_PREP(AD5592R_DATA_MSK, data);

	put_unaligned_be16(tx, &packed);

	dev_info(&st->spi->dev, "SPI WRITE msg tx %x\n", tx);
	dev_info(&st->spi->dev, "SPI WRITE msg packed %x\n", packed);

	return spi_sync_transfer(st->spi, &t, 1);
}

static int ad5592r_s_spi_read(struct ad5592r_s_state *st,
			      u8 addr, u16 *data)
{
	u16 rx = 0;
	u16 reg_rdb_data;
	u16 recived_data = 0;
	int ret = 0;

	struct spi_transfer t = {
		.tx_buf = NULL,
		.rx_buf = &rx,
		.len = 2,
	};

	reg_rdb_data = FIELD_PREP(AD5592R_EN_READB, 1) |
			FIELD_PREP(AD5592R_S_REG_SELECT_RDB, addr);

	ret = ad5592r_s_spi_write(st, AD5592R_REG_RDB_ADDR, reg_rdb_data);
	if (ret){
		dev_info(&st->spi->dev, "Writing readback command failed\n");
		return ret;
	}

	ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret){
		dev_info(&st->spi->dev, "SPI transfer failed\n");
		return ret;
	}
	recived_data = get_unaligned_be16(&rx);
	*data = FIELD_GET(AD5592R_DATA_MSK, recived_data);

	return 0;
}

static int ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev,
					unsigned int reg,
					unsigned int writeval,
					unsigned int *readval)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	if(readval){
		return ad5592r_s_spi_read(st, reg, (u16 *)readval);
	}else{
		return ad5592r_s_spi_write(st, reg, writeval);
	}

	return 0;
}

#define AD5592R_S_CHANNEL(_channel)			\
	{						\
		.type = IIO_VOLTAGE,			\
		.indexed = 1,				\
		.channel = (_channel),			\
		.info_mask_separate =			\
			BIT(IIO_CHAN_INFO_RAW),		\
		.info_mask_shared_by_all =		\
			BIT(IIO_CHAN_INFO_ENABLE),	\
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
	.debugfs_reg_access = ad5592r_s_debugfs_reg_access,
};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ad5592r_s_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);

	st->reg_select = 1;
	memset(st->chan_val, 0, sizeof(st->chan_val));
	st->spi = spi;
	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channels);

	ad5592r_s_spi_write(st, AD5592R_REG_PD_ADDR, FIELD_PREP(AD5592R_REG_EN_IREF, 1)); /* Reset the device */

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