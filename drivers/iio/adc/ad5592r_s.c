// SPDX-License-Identifier: GPL-2.0
/*
 * IIO CORA SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 */

#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/unaligned.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

#define AD5592R_S_MSB_MSK			BIT(15)
#define AD5592R_REG_ADDR_MSK		GENMASK(14, 11)
#define AD5592R_REG_DATA_MSK		GENMASK(10, 0)

#define AD5592R_REG_READ_AND_LDAC	0x07
#define AD5592R_REG_RD_EN_MSK		BIT(6)
#define AD5592R_REG_RD_ADDR_MSK		GENMASK(5, 2)

#define AD5592R_REG_PD_REF_CTRL		0x0B
#define AD5592R_POWER_ENABLE		0x000
#define AD5592R_POWER_DISABLE		BIT(10)
#define AD5592R_REG_EN_IREF			BIT(9)

struct iio_adc_cora_st {
	struct spi_device *spi;
	int reg_select;
	int chan_val[6];
};

static int iio_adc_cora_spi_write(struct iio_adc_cora_st *st, u8 addr, u16 data)
{
	u16 tx = 0;
	u16 package = 0;

	struct spi_transfer t = {
		.tx_buf = &package,
		.len = 2
	};

	tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) | FIELD_PREP(AD5592R_REG_ADDR_MSK, addr) | FIELD_PREP(AD5592R_REG_DATA_MSK, data);
	put_unaligned_be16(tx, &package);

	dev_info(&st->spi->dev, "tx we constructed %x\n", tx);
	dev_info(&st->spi->dev, "package we constructed %x\n", package);

	return spi_sync_transfer(st->spi, &t, 1);
}

static int iio_adc_cora_spi_read(struct iio_adc_cora_st *st, u8 addr, u16 *data)
{
	u16 rx = 0;
	u16 reg_rdb_data;
	u16 rcv_data;
	int ret = 0;

	struct spi_transfer t = {
		.tx_buf = NULL,
		.rx_buf = &rx,
		.len = 2
	};

	reg_rdb_data =  FIELD_PREP(AD5592R_REG_RD_EN_MSK, 1) |
					FIELD_PREP(AD5592R_REG_RD_ADDR_MSK, addr);
	ret = iio_adc_cora_spi_write(st, AD5592R_REG_READ_AND_LDAC, reg_rdb_data);
	if (ret) {
		dev_err(&st->spi->dev, "Writing the readback register failed %d\n", ret);
		return ret;
	}

	ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret) {
		dev_err(&st->spi->dev, "Failed receiving readback %d\n", ret);
		return ret;
	}

	rcv_data = get_unaligned_be16(&rx);
	*data = FIELD_GET(AD5592R_REG_DATA_MSK, rcv_data);
	return 0;
}

static int iio_adc_cora_debugfs_reg_access(struct iio_dev *indio_dev,
								unsigned int reg, unsigned writeval,
								unsigned int *readval)
{
	struct iio_adc_cora_st *st = iio_priv(indio_dev);

	if (readval)
		return iio_adc_cora_spi_read(st, reg, (u16*)readval);

	return iio_adc_cora_spi_write(st, reg, writeval);
}

static int iio_adc_cora_read_raw(struct iio_dev *indio_dev,
								struct iio_chan_spec const *chan,
								int *val,
								int *val2,
								long mask)
{
	struct iio_adc_cora_st *st = iio_priv(indio_dev);

	switch(mask) {
		case IIO_CHAN_INFO_RAW:
			if(!st->reg_select) {
				if (chan->channel == 0)
					*val = st->chan_val[0];
				else if (chan->channel == 1)
					*val = st->chan_val[1];
				else if (chan->channel == 2)
					*val = st->chan_val[2];
				else if (chan->channel == 3)
					*val = st->chan_val[3];
				else if (chan->channel == 4)
					*val = st->chan_val[4];
				else if (chan->channel == 5)
					*val = st->chan_val[5];
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

static int iio_adc_cora_write_raw(struct iio_dev *indio_dev,
								struct iio_chan_spec const *chan,
								int val,
								int val2,
								long mask)
{
	struct iio_adc_cora_st *st = iio_priv(indio_dev);

	switch(mask) {
		case IIO_CHAN_INFO_RAW:
			if(!st->reg_select) {
				if (chan->channel == 0) {
					dev_info(&indio_dev->dev, "Trying to write to channel 0: %d\n", val);
					st->chan_val[0] = val;
				}
				else if (chan->channel == 1) {
					dev_info(&indio_dev->dev, "Trying to write to channel 1: %d\n", val);
					st->chan_val[1] = val;
				}
				else if (chan->channel == 2) {
					dev_info(&indio_dev->dev, "Trying to write to channel 2: %d\n", val);
					st->chan_val[2] = val;
				}
				else if (chan->channel == 3) {
					dev_info(&indio_dev->dev, "Trying to write to channel 3: %d\n", val);
					st->chan_val[3] = val;
				}
				else if (chan->channel == 4) {
					dev_info(&indio_dev->dev, "Trying to write to channel 4: %d\n", val);
					st->chan_val[4] = val;
				}
				else if (chan->channel == 5) {
					dev_info(&indio_dev->dev, "Trying to write to channel 5: %d\n", val);
					st->chan_val[5] = val;
				}
				return 0;
			}
			else
				return -EINVAL;
		case IIO_CHAN_INFO_ENABLE:
			if (val)
				st->reg_select = 1;
			else
				st->reg_select = 0;
				return 0;
		default:
			return -EINVAL;
	}
}

static const struct iio_chan_spec iio_adc_cora_channels[] = {
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

static const struct iio_info iio_adc_cora_info = {
	.read_raw = &iio_adc_cora_read_raw,
	.write_raw = &iio_adc_cora_write_raw,
	.debugfs_reg_access = &iio_adc_cora_debugfs_reg_access
};

static int iio_adc_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_adc_cora_st *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->reg_select = 1;
	st->spi = spi;
	memset(st->chan_val, 0, sizeof(st->chan_val));

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &iio_adc_cora_info;
	indio_dev->channels = iio_adc_cora_channels;
	indio_dev->num_channels = ARRAY_SIZE(iio_adc_cora_channels);

	iio_adc_cora_spi_write(st, AD5592R_REG_PD_REF_CTRL,
											FIELD_PREP(AD5592R_REG_EN_IREF, 1));

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver iio_adc_driver = {
	.driver = {
		.name = "ad5592r_s"
	},
	.probe = iio_adc_probe
};

module_spi_driver(iio_adc_driver);

MODULE_AUTHOR("Moldovan Flavius <flaviuss3035@gmail.com>");
MODULE_DESCRIPTION("Analog Devices IIO CORA Summer School");
MODULE_LICENSE("GPL");
