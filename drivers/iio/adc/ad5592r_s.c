// SPDX-License-Identifier: GPL-2.0
/*
	* IIO SPI ADC driver
	*
	* Copyright 2026 Analog Devices Inc.
	*/
#include <linux/bitfield.h>
#include <linux/unaligned.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/delay.h>

#define AD5592R_S_MSB_MSK BIT(15)
#define AD5592R_ADDR_MSK GENMASK(14, 11)
#define AD5592R_DATA_MSK GENMASK(10, 0)

#define AD5592R_REG_RDB_ADDR 0x7
#define AD5592_EN_READB BIT(6)
#define AD5592R_REG_SELECT_RDB GENMASK(5, 2)

#define AD5592R_REG_PD_ADDR 0xB
#define AD5592R_REG_EN_IREF BIT(9)

#define AD5592R_ADC_DATA_MSK GENMASK(11, 0)

#define AD5592R_ADC_ADDR_MSK GENMASK(14, 12)
#define AD5592R_REG_ADC_SEQ 0x02

#define AD5592R_REG_NOP 0x00

#define AD5592R_REG_ADC_CONFIG 0x04 //pentru configurarea piniilor de ADC
struct ad5592r_s_st {
	bool reg_select;
	int chan_val[6];
	struct spi_device *spi;
};
static int ad5592r_s_spi_write(struct ad5592r_s_st *st, u8 addr, u16 data)
{
	u16 tx = 0;
	u16 package = 0;
	struct spi_transfer t = { .tx_buf = &package, .len = 2 };

	tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) |
	     FIELD_PREP(AD5592R_ADDR_MSK, addr) |
	     FIELD_PREP(AD5592R_DATA_MSK, data);
	put_unaligned_be16(tx, &package);
	dev_info(&st->spi->dev, "Constructed TX:%x \nConstructed PACKAGE:%x",
		 tx, package);
	return spi_sync_transfer(st->spi, &t, 1);
}
static int ad5592r_s_spi_read(struct ad5592r_s_st *st, u8 addr, u16 *data)
{
	u16 rx = 0;
	u16 reg_rdb_data;
	u16 rcv_data = 0;
	int ret = 0;
	struct spi_transfer t = { .tx_buf = NULL, .rx_buf = &rx, .len = 2 };
	reg_rdb_data = FIELD_PREP(AD5592_EN_READB, 1) |
		       FIELD_PREP(AD5592R_REG_SELECT_RDB, addr);
	ret = ad5592r_s_spi_write(st, AD5592R_REG_RDB_ADDR, reg_rdb_data);
	if (ret)
		return ret;

	ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret)
		return ret;

	rcv_data = get_unaligned_be16(&rx);
	*data = FIELD_GET(AD5592R_DATA_MSK, rcv_data);
	return 0;
}
static int ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev, unsigned reg,
					unsigned writeval, unsigned *readval)
{
	struct ad5592r_s_st *st = iio_priv(indio_dev);

	if (readval) {
		return ad5592r_s_spi_read(st, reg, (u16 *)readval);
	}
	return ad5592r_s_spi_write(st, reg, writeval);
}

static int ad5592r_s_read_chan(struct ad5592r_s_st *st, int channel,
			       u16 *readval)
{
	u8 chan = (u8)channel;
	u16 data, rx = 0;
	struct spi_transfer t = { .tx_buf = NULL, .rx_buf = &rx, .len = 2 };
	int ret; //MSB ADDR DATA

	ret = ad5592r_s_spi_write(
		st, AD5592R_REG_ADC_SEQ,
		BIT(chan)); // ADC SEQ este secventa pentru selectarea ADC-ului
	// selectam canalul chan
	if (ret) {
		return ret;
	}

	udelay(1); //delay o microsecunda

	ret = ad5592r_s_spi_write(st, AD5592R_REG_NOP, 0); //no operation
	if (ret) {
		return ret;
	}

	ret = spi_sync_transfer(st->spi, &t,
				1); //transferul de date din SDO 2 bytes in rx
	if (ret) {
		return ret;
	}

	data = get_unaligned_be16(&rx);
	*readval = FIELD_GET(AD5592R_ADC_DATA_MSK, data);
	return 0;
}

static irqreturn_t ad5592r_s_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct ad5592r_s_st *st = iio_priv(indio_dev);

	int bit;
	int ret;
	int i = 0;

	u16 buf[6];

	for_each_set_bit(bit, indio_dev->active_scan_mask,
			 indio_dev->num_channels) {
		ret = ad5592r_s_read_chan(st, bit, &buf[i]);

		if (ret) {
			dev_err(&st->spi->dev,
				"Reading channel %d failed in trigger: %d\n",
				bit, ret);

			iio_trigger_notify_done(indio_dev->trig);

			return IRQ_HANDLED;
		}

		i++;
	}

	iio_push_to_buffers(indio_dev, buf);

	iio_trigger_notify_done(indio_dev->trig);

	return IRQ_HANDLED;
}

struct iio_adc_st {
	bool reg_select;
	int chan_val[6];
};

static const struct iio_chan_spec iio_adc_channels[] = {
	{ .type = IIO_VOLTAGE,
	  .channel = 0,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .scan_index = 0,
		.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 16,
			.shift = 0,
			.endianness = IIO_CPU,
		}	
 },
	{ .type = IIO_VOLTAGE,
	  .channel = 1,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .scan_index = 1,
		.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 16,
			.shift = 0,
			.endianness = IIO_CPU,
		}	
	 },
	{ .type = IIO_VOLTAGE,
	  .channel = 2,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .scan_index = 2,
		.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 16,
			.shift = 0,
			.endianness = IIO_CPU,
		}	
	 },
	{ .type = IIO_VOLTAGE,
	  .channel = 3,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .scan_index = 3,
		.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 16,
			.shift = 0,
			.endianness = IIO_CPU,
		}	
	},
	{ .type = IIO_VOLTAGE,
	  .channel = 4,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .scan_index = 4,
		.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 16,
			.shift = 0,
			.endianness = IIO_CPU,
		}	
	 },
	{ .type = IIO_VOLTAGE,
	  .channel = 5,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .scan_index = 5,
		.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 16,
			.shift = 0,
			.endianness = IIO_CPU,
		}	
	 },
};

static int iio_adc_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan, int *val,
			    int *val2, long mask)
{
	struct ad5592r_s_st *st = iio_priv(indio_dev);
	u16 adc_val = 0;
	// citim datele
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		// citim datele de la ADC
		if (!st->reg_select) {
			ad5592r_s_read_chan(st, chan->channel, &adc_val);
			*val = adc_val;
		} else {
			return -EINVAL;
		}
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select ? 1 : 0;
		return IIO_VAL_INT;

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

static const struct iio_info ad5592r_info = {
	.read_raw = &iio_adc_read_raw,
	.write_raw = &iio_adc_write_raw,
	.debugfs_reg_access = &ad5592r_s_debugfs_reg_access
};
static int ad5592r_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ad5592r_s_st *st;
	int ret;

	//pentru alocarea memoriei
	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->reg_select = true;
	st->spi = spi;
	memset(st->chan_val, 0, sizeof(st->chan_val));
	//popularea structurii
	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_info;
	indio_dev->channels = iio_adc_channels;
	indio_dev->num_channels = 6;
	indio_dev->modes = INDIO_DIRECT_MODE;

	ad5592r_s_spi_write(st, AD5592R_REG_PD_ADDR,
			    FIELD_PREP(AD5592R_REG_EN_IREF, 1));
	ret = ad5592r_s_spi_write(st, AD5592R_REG_ADC_CONFIG,
				  0x3F); // configuram pinii IO0-5 sa fie de ADC
	if (ret)
		return ret;

	ret = devm_iio_triggered_buffer_setup(&spi->dev, indio_dev, NULL,
					      &ad5592r_s_trigger_handler, NULL);

	if (ret) {
		dev_err(&spi->dev, "Failed to create triggered buffer\n");
		return ret;
	}

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_driver = { .driver = { .name = "ad5592r_s" },
					    .probe = ad5592r_probe };

module_spi_driver(ad5592r_driver);
MODULE_AUTHOR("Tomescu Lucas <lucastomescu@gmail.com>");
MODULE_DESCRIPTION(
	"Analog Devices AD5592R_S IIO SPI ADC driver for Summer School");
MODULE_LICENSE("GPL");
