// SPDX-License-Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2026 Trif Marius Andrei
 */

#include <linux/unaligned.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/bitfield.h>

#define ADC_RDWR_MSK BIT(15)
#define ADC_ADDR_MSK GENMASK(14, 11)
#define ADC_DATA_MSK GENMASK(10, 0)
#define ADC_REG_EN_IREF BIT(9)
#define ADC_REG_POWER_ADDRES 0xB

#define ADC_REG_RDB_ADDR 0x7
#define ADC_EN_BIT_MSK BIT(6)
#define ADC_REG_RD_MSK GENMASK(5, 2)

#define ADC_POWER_REG 0x02
#define ADC_POWER_ENABLE 0x0
#define ADC_POWER_DISABLE 0x20

#define ADC_REG_ADDRESS_SET_ADC 0b0100
#define ADC_REG_ADDRESS_SET_SEQ 0b0010

#define ADC_REG_CHAN_SEL_ADC GENMASK(5, 0)
#define ADC_REG_ALL_BITS_MASK GENMASK(15, 0)
#define ADC_REG_CHAN_PUT_SEQ(x) BIT(x)

#define ADC_REG_SELECT11_0B GENMASK(11, 0)
#define ADC_REG_CHAN_SEL(x) (0x0 + (x))
#define ADC_RESULT_VALUE GENMASK(11, 0)

struct iio_adc_placa_st {
	struct spi_device *spi;
	bool reg_select;
	int chan_val[6];
};

static int iio_adc_placa_spi_write(struct iio_adc_placa_st *st, u8 addr,
				   u16 data)
{
	u16 tx = 0;
	u16 package = 0;

	struct spi_transfer t = {
		.tx_buf = &package,
		.len = 2,
	};

	tx = FIELD_PREP(ADC_RDWR_MSK, 0) | FIELD_PREP(ADC_ADDR_MSK, addr) |
	     FIELD_PREP(ADC_DATA_MSK, data);

	put_unaligned_be16(tx, &package);

	dev_info(&st->spi->dev, "tx we constructed %x\n", tx);
	dev_info(&st->spi->dev, "package we constructed %x\n", package);

	return spi_sync_transfer(st->spi, &t, 1);
}

static int iio_adc_placa_spi_read(struct iio_adc_placa_st *st, u8 addr,
				  u16 *data)
{
	u16 reg_rdb_data = 0;
	u16 rx = 0;
	int ret = 0;
	u16 rcv_data = 0;

	struct spi_transfer t = { .tx_buf = NULL, .rx_buf = &rx, .len = 2 };

	reg_rdb_data = FIELD_PREP(ADC_EN_BIT_MSK, 1) |
		       FIELD_PREP(ADC_REG_RD_MSK, addr);
	ret = iio_adc_placa_spi_write(st, ADC_REG_RDB_ADDR, reg_rdb_data);

	if (ret) {
		dev_info(&st->spi->dev, "Spi readback transfer failed %d\n",
			 ret);
		return ret;
	}

	ret = spi_sync_transfer(st->spi, &t, 1);

	if (ret) {
		dev_info(&st->spi->dev,
			 "Spi receiving transfer readback failed %d\n", ret);
		return ret;
	}

	rcv_data = get_unaligned_be16(&rx);
	*data = FIELD_GET(ADC_DATA_MSK, rcv_data);

	return 0;
}

static int iio_adc_placa_debugfs_reg_access(struct iio_dev *indio_dev,
					    unsigned reg, unsigned writeval,
					    unsigned *readval)
{
	struct iio_adc_placa_st *st = iio_priv(indio_dev);
	if (readval)
		return iio_adc_placa_spi_read(st, reg, (u16 *)readval);
	return iio_adc_placa_spi_write(st, reg, writeval);
}

static int iio_adc_placa_read_channel(struct iio_adc_placa_st *st, int channel,
				      u16 *readval)
{
	int ret = iio_adc_placa_spi_write(st, ADC_REG_ADDRESS_SET_SEQ,
					  ADC_REG_CHAN_PUT_SEQ(channel));

	if (ret) {
		dev_err(&st->spi->dev, "Writing SEQ conversion failed %d\n",
			ret);
		return ret;
	}

	u16 rx = 0;
	struct spi_transfer t = {
		.rx_buf = &rx,
		.len = 2,
	};

	ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret) {
		dev_err(&st->spi->dev, "Dummy transfer failed %d\n", ret);
		return ret;
	}

	ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret) {
		dev_err(&st->spi->dev, "Data transfer failed %d\n", ret);
		return ret;
	}

	u16 rcv_data = get_unaligned_be16(&rx);
	*readval = FIELD_GET(ADC_RESULT_VALUE, rcv_data);

	return 0;
}

static int iio_adc_placa_read_raw(struct iio_dev *indio_dev,
				  struct iio_chan_spec const *chan, int *val,
				  int *val2, long mask)
{
	struct iio_adc_placa_st *st = iio_priv(indio_dev);

	int ret = 0;
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			ret = iio_adc_placa_read_channel(st, chan->channel,
							 (u16 *)val);
			if (ret) {
				dev_err(&st->spi->dev,
					"Reading from channels failed %d\n",
					ret);
				return ret;
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

static int iio_adc_placa_write_raw(struct iio_dev *indio_dev,
				   struct iio_chan_spec const *chan, int val,
				   int val2, long mask)
{
	struct iio_adc_placa_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			if (chan->channel == 0) {
				dev_info(&indio_dev->dev,
					 "Trying to write channel 0");
				// st->chan_val[0] = val;
			}
			if (chan->channel == 1) {
				dev_info(&indio_dev->dev,
					 "Trying to write channel 1");
				// st->chan_val[1] = val;
			}
			if (chan->channel == 2) {
				dev_info(&indio_dev->dev,
					 "Trying to write channel 2");
				// st->chan_val[2] = val;
			}
			if (chan->channel == 3) {
				dev_info(&indio_dev->dev,
					 "Trying to write channel 3");
				// st->chan_val[3] = val;
			}
			if (chan->channel == 4) {
				dev_info(&indio_dev->dev,
					 "Trying to write channel 4");
				// st->chan_val[4] = val;
			}
			if (chan->channel == 5) {
				dev_info(&indio_dev->dev,
					 "Trying to write channel 5");
				// st->chan_val[5] = val;
			}
			return 0;
		} else
			return -EINVAL;
	case IIO_CHAN_INFO_ENABLE:
		if (val) {
			st->reg_select = true;
			iio_adc_placa_spi_write(st, ADC_POWER_REG,
						ADC_POWER_DISABLE);
		} else {
			st->reg_select = false;
			iio_adc_placa_spi_write(st, ADC_POWER_REG,
						ADC_POWER_ENABLE);
		}
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct iio_chan_spec iio_adc_placa_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	}
};

static const struct iio_info iio_adc_placa_info = {
	.read_raw = &iio_adc_placa_read_raw,
	.write_raw = &iio_adc_placa_write_raw,
	.debugfs_reg_access = &iio_adc_placa_debugfs_reg_access,
};

///Tot timpul se incepe cu functia de probe

///folosim static pentru ca linux e destul de mare si de vast si sunt sanse sa
///scrie 2 persoane cu acelasi nume. si ca sa fim siguri
///ca ne apelam functia noastra o scriem static.
static int iio_adc_placa_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;

	struct iio_adc_placa_st *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->spi = spi;

	memset(st->chan_val, 0, sizeof(st->chan_val));
	st->reg_select = true;
	iio_adc_placa_spi_write(st, ADC_REG_POWER_ADDRES,
				FIELD_PREP(ADC_REG_EN_IREF, 1));
	indio_dev->name = "iio_adc_placa";
	indio_dev->info = &iio_adc_placa_info;
	indio_dev->channels = iio_adc_placa_channels;
	indio_dev->num_channels = ARRAY_SIZE(iio_adc_placa_channels);

	int ret = iio_adc_placa_spi_write(st, ADC_REG_ADDRESS_SET_ADC,
					  ADC_REG_CHAN_SEL_ADC);
	if (ret) {
		dev_err(&st->spi->dev, "Writing START conversion failed %d\n",
			ret);
		return ret;
	}

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver iio_adc_placa_driver = {
	.driver = { .name = "iio_adc_placa" },
	.probe = iio_adc_placa_probe
};

module_spi_driver(iio_adc_placa_driver);

MODULE_AUTHOR("Marius Trif <mariustrif0323@yahoo.com");
MODULE_DESCRIPTION("Analog Devices IIO-EMU SPI ADC Summer School");
MODULE_LICENSE("GPL v2");
