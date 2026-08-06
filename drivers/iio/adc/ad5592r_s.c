// SPDX-License-Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2026 Trif Marius Andrei
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

struct iio_adc_placa_st {
	bool reg_select;
	int chan_val[6];
};

static int iio_adc_placa_read_raw(struct iio_dev *indio_dev,
				  struct iio_chan_spec const *chan, int *val,
				  int *val2, long mask)
{
	struct iio_adc_placa_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if(!st->reg_select){
			if (chan->channel == 0)
				*val = st->chan_val[0];
			if (chan->channel == 1)
				*val = st->chan_val[1];
			if (chan->channel == 2)
				*val = st->chan_val[2];
			if (chan->channel == 3)
				*val = st->chan_val[3];
			if (chan->channel == 4)
				*val = st->chan_val[4];
			if (chan->channel == 5)
				*val = st->chan_val[5];
			return IIO_VAL_INT;
		}else
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
		if(!st->reg_select){
			if (chan->channel == 0){
				dev_info(&indio_dev->dev, 
					"Trying to write channel 0");
				st->chan_val[0] = val;
			}
			if (chan->channel == 1){
				dev_info(&indio_dev->dev, 
					"Trying to write channel 1");
				st->chan_val[1] = val;
			}
			if (chan->channel == 2){
				dev_info(&indio_dev->dev, 
					"Trying to write channel 2");
				st->chan_val[2] = val;
			}
			if (chan->channel == 3){
				dev_info(&indio_dev->dev, 
					"Trying to write channel 3");
				st->chan_val[3] = val;
			}
			if (chan->channel == 4){
				dev_info(&indio_dev->dev, 
					"Trying to write channel 4");
				st->chan_val[4] = val;
			}
			if (chan->channel == 5){
				dev_info(&indio_dev->dev, 
					"Trying to write channel 5");
				st->chan_val[5] = val;
			}
			return 0;
		}else
			return -EINVAL;
	case IIO_CHAN_INFO_ENABLE:
		if(val)
			st->reg_select = true;
		else
			st->reg_select = false;	
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
	.write_raw = &iio_adc_placa_write_raw
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
	memset(st->chan_val, 0, sizeof(st->chan_val));
	st->reg_select = true;
	indio_dev->name = "iio_adc_placa";
	indio_dev->info = &iio_adc_placa_info;
	indio_dev->channels = iio_adc_placa_channels;
	indio_dev->num_channels = ARRAY_SIZE(iio_adc_placa_channels);

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
