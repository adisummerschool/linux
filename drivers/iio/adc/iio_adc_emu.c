// SPDX-License Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2026 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

struct iio_adc_emu_st {
	bool reg_select;
	int chan_val[2];
};

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
		return 0;
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
		st->reg_select = val ? true : false;
		return 0;
	default:
		return -EINVAL;
	}
	return 0;
	//returnam datele scrise
}
static const struct iio_info iio_adc_emu_info = {
	.read_raw = &iio_adc_emu_read_raw,
	.write_raw = &iio_adc_emu_write_raw
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
