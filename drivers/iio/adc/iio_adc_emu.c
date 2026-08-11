// SPDX-License Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2026 Analog Devices Inc.
 */

#include <linux/unaligned.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/bitfield.h>

#define EMU_RDWR_MASK BIT(7)
#define EMU_ADDR_MSK GENMASK(15, 8)
#define EMU_DATA_MSK GENMASK(7, 0)
#define EMU_POWER_REG 0x02
#define EMU_POWER_ENABLE 0x0
#define EMU_POWER_DISABLE 0x20

struct iio_adc_emu_st {
	struct spi_device *spi;
	bool reg_select;
	int chan_val[2];
};

static int iio_adc_emu_spi_read(struct iio_adc_emu_st *st, u8 addr, u8 *data)
{
	u8 tx = 0, rx = 0;
	int ret = 0;
	struct spi_transfer t[] = { { .tx_buf = &tx, .len = 1 },
				    { .rx_buf = &rx, .len = 1 } };

	tx = FIELD_PREP(EMU_RDWR_MASK, 1) | addr;

	ret = spi_sync_transfer(st->spi, t, 2);
	if (ret) {
		dev_info(&st->spi->dev, "SPI read failed");
		return ret;
	}

	*data = rx;
	return 0;
}

static int iio_adc_emu_spi_write(struct iio_adc_emu_st *st, u8 addr, u8 data)
{
	u16 tx = 0;
	u16 package = 0;
	struct spi_transfer t = { .tx_buf = &package, .len = 2 };

	tx = FIELD_PREP(EMU_RDWR_MASK, 0) | FIELD_PREP(EMU_ADDR_MSK, addr) |
	     FIELD_PREP(EMU_DATA_MSK, data);
	put_unaligned_be16(tx, &package);

	dev_info(&st->spi->dev,
		 "Constructed TX:0x%02x \nConstructed PACKAGE:0x%02x", tx,
		 package);

	return spi_sync_transfer(st->spi, &t, 1);
}

static int iio_adc_emu_debugfs_reg_access(struct iio_dev *indio_dev,
					  unsigned reg, unsigned writeval,
					  unsigned *readval)
{
	struct iio_adc_emu_st *st = iio_priv(indio_dev);

	if (readval) {
		return iio_adc_emu_spi_read(st, reg, (u8 *)readval);
	}
	return iio_adc_emu_spi_write(st, reg, writeval);
}

// structura pentru canalele ADC
static const struct iio_chan_spec iio_adc_emu_channels[] = {
	{ .type = IIO_VOLTAGE,
	  .channel = 0,
	  .indexed = 1,
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) },
	{ .type = IIO_VOLTAGE,
	  .channel = 1,
	  .indexed = 1,
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) }
};

// citim datele de la ADC
static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan, int *val,
				int *val2, long mask)
{
	// citim datele
	struct iio_adc_emu_st *st = iio_priv(indio_dev);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		// citim datele de la ADC
		if (!st->reg_select) {
			if (chan->channel) {
				//channel 1
				*val = st->chan_val[1]; //hardcoded value for channel 1
			} else {
				//channel 0
				*val = st->chan_val[0]; //hardcoded value for channel 0
			}
			return IIO_VAL_INT;
		} else {
			return -EINVAL;
		}
	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select ? 1 : 0;
		return IIO_VAL_INT;
	default:
		//returnam datele citite
		return -EINVAL;
	}
	return 0;
}

static int iio_adc_emu_write_raw(struct iio_dev *indio_dev,
				 struct iio_chan_spec const *chan, int val,
				 int val2, long mask)
{
	// scriem datele
	struct iio_adc_emu_st *st = iio_priv(indio_dev);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			if (chan->channel) {
				st->chan_val[1] = val;
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 1");
			} else {
				st->chan_val[0] = val;
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 0");
			}
			return 0;
		} else {
			return -EINVAL;
		}
	case IIO_CHAN_INFO_ENABLE:
		if(val){
		st->reg_select = 1;
		iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_DISABLE);
		}else{
		st->reg_select = 0;
		iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_ENABLE);
		}

		return 0;
	default:
		return -EINVAL;
	}
	return 0;
	//returnam datele scrise
}
static const struct iio_info iio_adc_emu_info = {
	.read_raw = &iio_adc_emu_read_raw,
	.write_raw = &iio_adc_emu_write_raw,
	.debugfs_reg_access = iio_adc_emu_debugfs_reg_access
};
// initializam driverul
static int iio_adc_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_adc_emu_st *st;

	//pentru alocarea memoriei
	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->reg_select = true;
	st->spi = spi;
	memset(st->chan_val, 0, sizeof(st->chan_val));
	//popularea structurii
	indio_dev->name = "iio_adc_emu";
	indio_dev->info = &iio_adc_emu_info;
	indio_dev->channels = iio_adc_emu_channels;
	indio_dev->num_channels = 2;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

// definim driverul SPI
static struct spi_driver iio_adc_emu_driver = {
	.driver = { .name = "iio_adc_emu" },
	.probe = iio_adc_emu_probe
};
module_spi_driver(iio_adc_emu_driver);

MODULE_AUTHOR("Tomescu Lucas <lucastomescu@gmail.com>");
MODULE_DESCRIPTION("Analog Devices IIO ADC EMU Summer School");
MODULE_LICENSE("GPL v2");
