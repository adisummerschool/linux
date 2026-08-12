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
#include <linux/unaligned.h>
#include <linux/bitfield.h>


const int CHANEL_0 = 0, CHANEL_1 = 1, CHANEL_2 = 2;
const int CHANEL_3 = 3, CHANEL_4 = 4, CHANEL_5 = 5;

#define AD5592R_S_CONTROL_MASK	BIT(15)

#define AD5592R_S_READB_REG	0x7
#define AD5592R_S_READ_EN_MASK	BIT(6)
#define AD5592R_S_READB_ADDR_MASK	GENMASK(5,2)

#define AD5592R_S_REG_ADDR_MASK	GENMASK(14,11)
#define AD5592R_S_DATA_MASK	GENMASK(10,0)

#define AD5592R_S_POWER_ON_REG	0xB
#define AD5592R_S_POWER_EN	BIT(9)

#define AD5592R_S_ADC_SEQ_REG	0x02
#define AD5592R_S_ADC_SEQ_EN(x)	1<<x

#define AD5592R_S_PLLDWN_REG_ADDR	0x06
#define AD5592R_S_ADC_CONF_REG	0x04
#define AD5592R_S_ADC_CONF_MASK	GENMASK(5,0)

#define AD5592R_S_ADC_ADDR_MASK	GENMASK(14,12)
#define AD5592R_S_ADC_DATA_MASK	GENMASK(11,0)



struct iio_ad5592r_s_st {
	bool reg_select;
	int chan_val[6];
	struct spi_device *spi;
};

static int iio_ad5592r_s_spi_write(struct iio_ad5592r_s_st *st, u8 addr, u16 data)
{
	u16 tx = 0;
	u16 package = 0;

	struct spi_transfer t= {
		.tx_buf = &package,
		.len = 2
	};

	tx = FIELD_PREP(AD5592R_S_CONTROL_MASK,0) |
		FIELD_PREP(AD5592R_S_REG_ADDR_MASK,addr) |
		FIELD_PREP(AD5592R_S_DATA_MASK,data);

	put_unaligned_be16(tx,&package);

	dev_info(&st->spi->dev,"Tx constructed: %x\n",tx);
	dev_info(&st->spi->dev,"Package we transferred: %x\n",package);

	return spi_sync_transfer(st->spi,&t,1);
}

static int iio_ad5592r_s_spi_read(struct iio_ad5592r_s_st *st, u8 addr,
				u16 *data)
{
	u16 rx = 0;
	u16 reg_rdb_data = 0;
	u16 data_buf = 0;
	int ret = 0;

	struct spi_transfer t = {
		.tx_buf = NULL,
		.rx_buf = &rx,
		.len = 2
	};

	reg_rdb_data = FIELD_PREP(AD5592R_S_READ_EN_MASK,1) |
			FIELD_PREP(AD5592R_S_READB_ADDR_MASK,addr);

	ret = iio_ad5592r_s_spi_write(st,AD5592R_S_READB_REG,reg_rdb_data);
	if(ret)
	{
		dev_err(&st->spi->dev,"Writing the readback register failed");
		return ret;
	}

	ret = spi_sync_transfer(st->spi,&t,1);
	if(ret){
		dev_err(&st->spi->dev,"SPI read transfer failed %d\n",ret);
		return ret;
	}

	data_buf = get_unaligned_be16(&rx);
	*data = FIELD_GET(AD5592R_S_DATA_MASK,data_buf);
	return 0;
}

static int iio_ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev,
				  unsigned reg, unsigned writeval,
				  unsigned *readval)
{
	struct iio_ad5592r_s_st *st= iio_priv(indio_dev);

	if(readval)
		return iio_ad5592r_s_spi_read(st,reg,(u16*)readval);

	return iio_ad5592r_s_spi_write(st,reg,writeval);
}

static int iio_ad5592r_s_chan_read(struct iio_ad5592r_s_st *st, int chan,
				u16 *data)
{
	u16 rx= 0;
	u16 data_x = 0;
	u8 chan_check = 0;
	int ret = 0;

	struct spi_transfer t = {
		.tx_buf = NULL,
		.rx_buf = &rx,
		.len = 2
	};

	ret = iio_ad5592r_s_spi_write(st,AD5592R_S_ADC_SEQ_REG,
				FIELD_PREP(AD5592R_S_ADC_CONF_MASK,
					AD5592R_S_ADC_SEQ_EN(chan)));
	if(ret){
		dev_err(&st->spi->dev,
			"Writing to sequence register failed: %d\n", ret);
		return ret;
	}

	ret = iio_ad5592r_s_spi_write(st,0x0,0x0);
	if(ret){
		dev_err(&st->spi->dev,"Somehow NOP failed: %d\n", ret);
		return ret;
	}

	ret = spi_sync_transfer(st->spi,&t,1);
	if(ret){
		dev_err(&st->spi->dev,"SPI ADC read transfer failed %d\n",ret);
		return ret;
	}

	data_x = get_unaligned_be16(&rx);
	chan_check = FIELD_GET(AD5592R_S_ADC_ADDR_MASK,data_x);
	if (chan_check != chan){
		dev_err(&st->spi->dev,
			"Reading %d channel insted of %d ADC channel\n",
			chan_check,chan);
		return chan_check;
	}

	*data = FIELD_GET(AD5592R_S_ADC_DATA_MASK,data_x);

	return 0;
}

static int iio_ad5592r_s_io_config_adc(struct iio_ad5592r_s_st *st)
{
	int ret = 0;

	ret = iio_ad5592r_s_spi_write(st,AD5592R_S_PLLDWN_REG_ADDR,
				FIELD_PREP(AD5592R_S_ADC_CONF_MASK,0x0));
	if(ret){
		dev_err(&st->spi->dev,
			"Resetting I/O pin pulldown config failed: %d\n",ret);
		return ret;
	}
	ret = iio_ad5592r_s_spi_write(st,AD5592R_S_ADC_CONF_REG,
				FIELD_PREP(AD5592R_S_ADC_CONF_MASK,0x3F));
	if(ret){
		dev_err(&st->spi->dev,
			"Setting I/O pin ADC config failed: %d\n",ret);
		return ret;
	}

	return 0;
}

static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev,
				  struct iio_chan_spec const *chan, int *val,
				  int *val2, long mask)
{
	struct iio_ad5592r_s_st *st = iio_priv(indio_dev);

	int ret = 0;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			ret = iio_ad5592r_s_chan_read(st,chan->channel,
							(u16 *)val);
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
				dev_info(&indio_dev->dev,
					"Trying towrite to chanel 0");
				//st->chan_val[CHANEL_0] = val;
				break;
			case CHANEL_1:
				dev_info(&indio_dev->dev,
					"Trying to write to chanel 1");
				//st->chan_val[CHANEL_1] = val;
				break;
			case CHANEL_2:
				dev_info(&indio_dev->dev,
					"Trying to write to chanel 2");
				//st->chan_val[CHANEL_2] = val;
				break;
			case CHANEL_3:
				dev_info(&indio_dev->dev,
					"Trying to write to chanel 3");
				//st->chan_val[CHANEL_3] = val;
				break;
			case CHANEL_4:
				dev_info(&indio_dev->dev,
					"Trying to write to chanel 4");
				//st->chan_val[CHANEL_4] = val;
				break;
			case CHANEL_5:
				dev_info(&indio_dev->dev,
					"Trying to write to chanel 5");
				//st->chan_val[CHANEL_5] = val;
				break;
			default:
				return -EINVAL;
			}
                        return 0;
		} else
			return -EINVAL;
	case IIO_CHAN_INFO_ENABLE:
		if(val){
			st->reg_select = 1;
			iio_ad5592r_s_spi_write(st,AD5592R_S_POWER_ON_REG,
						FIELD_PREP(AD5592R_S_POWER_EN,0));
		}
		else{
			st->reg_select = 0;
			iio_ad5592r_s_spi_write(st,AD5592R_S_POWER_ON_REG,
						FIELD_PREP(AD5592R_S_POWER_EN,1));
		}
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
	.debugfs_reg_access = &iio_ad5592r_s_debugfs_reg_access,
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
	st->spi = spi;

	iio_ad5592r_s_io_config_adc(st);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

struct spi_driver iio_ad5592r_s_driver = { .driver = { .name = "iio_ad5592r_s" },
					   .probe = iio_ad5592r_s_probe };
module_spi_driver(iio_ad5592r_s_driver);

MODULE_AUTHOR("Papp Richard <pappr805@gmail.com>");
MODULE_DESCRIPTION("ADC driver for COraz7s board");
MODULE_LICENSE("GPL v2");