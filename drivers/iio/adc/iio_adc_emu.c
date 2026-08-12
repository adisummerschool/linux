// SPDX-License-Identifier: GPL-2.0
/*
 * AD5592r SPI ADC driver emulator
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2019 Renato Lui Geh
 */

#include <linux/unaligned.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/bitfield.h>

#define EMU_RW_MASK	BIT(7)
#define EMU_ADDR_MASK	GENMASK(14,8)
#define EMU_DATA_MASK	GENMASK(7,0)

#define EMU_POWER_ON_REG	0x02
#define EMU_POWER_EN	0x0
#define EMU_POWER_DIS	0x20

#define EMU_REG_CONVST	0x03
#define EMU_CONVST_START	BIT(0)
#define EMU_REG_CHAN_H(x)	(0x04 + (2 * (x)))
#define EMU_REG_CHAN_L(x)	(0x05 + (2 * (x)))

#define EMU_HIGH_DATA_MASK	GENMASK(11,8)

struct iio_adc_emu_st {
	bool reg_select;
	int chan_val[2];
	struct spi_device *spi;
};

static int iio_adc_emu_spi_read(struct iio_adc_emu_st *st, u8 addr,
				u8 *data)
{
	u8 tx = 0;
	u8 rx = 0;
	int ret = 0;

	struct spi_transfer t[] = {
		{
			.tx_buf = &tx,
			.len = 1
		},
		{
			.rx_buf = &rx,
			.len = 1
		}
	};

	tx = FIELD_PREP(EMU_RW_MASK,1) | addr;

	ret = spi_sync_transfer(st->spi,t,2);
	if(ret){
		dev_info(&st->spi->dev,"SPI read transfer failed %d\n",ret);
		return ret;
	}

	*data = rx;
	return 0;
}

static int iio_adc_emu_spi_write(struct iio_adc_emu_st *st, u8 addr, u8 data)
{
	u16 tx = 0;
	u16 package = 0;

	struct spi_transfer t= {
		.tx_buf = &package,
		.len = 2
	};

	tx = FIELD_PREP(EMU_RW_MASK,0) | FIELD_PREP(EMU_ADDR_MASK,addr)
		| FIELD_PREP(EMU_DATA_MASK, data);

	put_unaligned_be16(tx,&package);

	dev_info(&st->spi->dev,"Tx constructed: %x\n",tx);
	dev_info(&st->spi->dev,"Package we transferred: %x\n",package);

	return spi_sync_transfer(st->spi,&t,1);
}

static int iio_adc_emu_debugfs_reg_access(struct iio_dev *indio_dev,
				  unsigned reg, unsigned writeval,
				  unsigned *readval)
{
	struct iio_adc_emu_st *st= iio_priv(indio_dev);

	if(readval)
		return iio_adc_emu_spi_read(st,reg,(u8 *)readval);

	return iio_adc_emu_spi_write(st,reg,writeval);
}


static int iio_adc_emu_read_chan(struct iio_adc_emu_st *st, int chan,
				u16 *data)
{
	u8 data_high = 0;
	u8 data_low = 0;
	u16 data_x = 0;

	int ret = 0;

	ret = iio_adc_emu_spi_write(st,EMU_REG_CONVST,
					FIELD_PREP(EMU_CONVST_START,1));
	if(ret){
		dev_err(&st->spi->dev,"Writing conv reg failed: %d\n", ret);
		return ret;
	}

	ret = iio_adc_emu_spi_read(st,EMU_REG_CHAN_H(chan),&data_high);
	if(ret){
		dev_err(&st->spi->dev,"Reading high reg failed: %d\n", ret);
		return ret;
	}

	ret = iio_adc_emu_spi_read(st,EMU_REG_CHAN_L(chan),&data_low);
	if(ret){
		dev_err(&st->spi->dev,"Reading low reg failed: %d\n", ret);
		return ret;
	}

	data_x = FIELD_PREP(EMU_HIGH_DATA_MASK,data_high) | data_low;

	*data = data_x;

	return 0;
}

static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,
	                        struct iio_chan_spec const *chan, int *val,
	                        int *val2, long mask)
{
	struct iio_adc_emu_st *st = iio_priv(indio_dev);
	int ret = 0;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {

			ret = iio_adc_emu_read_chan(st,chan->channel,(u16 *)val);
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

static int iio_adc_emu_write_raw(struct iio_dev *indio_dev,
				 struct iio_chan_spec const *chan, int val,
				 int val2, long mask)
{
	struct iio_adc_emu_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			if (chan->channel) {
				dev_info(&indio_dev->dev,
					 "Trying to write to chanel 0");
				//st->chan_val[1] = val;
			} else {
				dev_info(&indio_dev->dev,
					 "Trying to write to chanel 1");
				//st->chan_val[0] = val;
			}
			return 0;
		} else
			return -EINVAL;
	case IIO_CHAN_INFO_ENABLE:
		if(val){
			st->reg_select = 1;
			iio_adc_emu_spi_write(st,EMU_POWER_ON_REG,EMU_POWER_DIS);
		}
		else{
			st->reg_select = 0;
			iio_adc_emu_spi_write(st,EMU_POWER_ON_REG,EMU_POWER_EN);
		}
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct iio_chan_spec iio_adc_emu_chanels[] = {
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
};

static const struct iio_info iio_adc_emu_info = {
	.read_raw = &iio_adc_emu_read_raw,
	.write_raw = &iio_adc_emu_write_raw,
	.debugfs_reg_access = &iio_adc_emu_debugfs_reg_access,
};

static int iio_adc_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_adc_emu_st *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	indio_dev->name = "iio_adc_emu";
	indio_dev->info = &iio_adc_emu_info;

	indio_dev->channels = iio_adc_emu_chanels;
	indio_dev->num_channels = ARRAY_SIZE(iio_adc_emu_chanels);

	st = iio_priv(indio_dev);
	st->reg_select = 1;
	memset(st->chan_val, 0, sizeof(st->chan_val));
	st->spi = spi;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

struct spi_driver iio_adc_emu_driver = { .driver = { .name = "iio_adc_emu" },
					 .probe = iio_adc_emu_probe };
module_spi_driver(iio_adc_emu_driver);

MODULE_AUTHOR("Papp Richard <pappr805@gmail.com>");
MODULE_DESCRIPTION("ADC driver emulator");
MODULE_LICENSE("GPL v2");