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

static int iio_adc_placa_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan, int *val,
				int *val2, long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (chan->channel == 0)
			*val = 0;
		if (chan->channel == 1)
            *val = 1;
        if (chan->channel == 2)
            *val = 2;
        if (chan->channel == 3)
            *val = 3;
        if (chan->channel == 4)
            *val = 4;
        if (chan->channel == 5)
            *val = 5;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int iio_adc_placa_write_raw(struct iio_dev *indio_dev,
				 struct iio_chan_spec const *chan, int val,
				 int val2, long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (chan->channel == 0)
			dev_info(&indio_dev->dev, "Trying to write channel 0");
		if (chan->channel == 1)
			dev_info(&indio_dev->dev, "Trying to write channel 1");
        if (chan->channel == 2)
			dev_info(&indio_dev->dev, "Trying to write channel 2");
        if (chan->channel == 3)
			dev_info(&indio_dev->dev, "Trying to write channel 3");
        if (chan->channel == 4)
			dev_info(&indio_dev->dev, "Trying to write channel 4");
        if (chan->channel == 5)
			dev_info(&indio_dev->dev, "Trying to write channel 5");	
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
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
    {
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
    {
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
    {
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
    {
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	}
};

static const struct iio_info iio_adc_placa_info = {
    .read_raw = &iio_adc_placa_read_raw,
    .write_raw = &iio_adc_placa_write_raw
};

///Tot timpul se incepe cu functia de probe

///folosim static pentru ca linux e destul de mare si de vast si sunt sanse sa scrie 2 persoane cu acelasi nume. si ca sa fim siguri
///ca ne apelam functia noastra o scriem static.
static int iio_adc_placa_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, 0);

	indio_dev->name = "iio_adc_placa";
	indio_dev->info = &iio_adc_placa_info;
    indio_dev->channels = iio_adc_placa_channels;
	indio_dev->num_channels = 6;

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
