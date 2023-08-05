// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for HDL Axi PWM LED
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#include <linux/iio/iio.h>

#define AXI_PWM_LED_CHANNEL_0		0x0042C
#define AXI_PWM_LED_CHANNEL_1		0x0046C
#define AXI_PWM_LED_CHANNEL_2		0x004AC
#define AXI_PWM_LED_CHANNEL_3		0x004EC
#define AXI_PWM_LED_CHANNEL_4		0x0052C
#define AXI_PWM_LED_CHANNEL_5		0x0056C

static struct axi_pwm_led_state {
	void __iomem	*regs;
};

static int axi_pwm_led_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan,
				int *val,
				int *val2,
				long mask)
{
	struct axi_pwm_led_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		*val = ioread16(st->regs + chan->address);
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int axi_pwm_led_write_raw(struct iio_dev *indio_dev,
				 struct iio_chan_spec const *chan,
				 int val,
				 int val2,
				 long mask)
{
	struct axi_pwm_led_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		iowrite32(val, st->regs + chan->address);
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct iio_info axi_pwm_led_info = {
	.read_raw = &axi_pwm_led_read_raw,
	.write_raw = &axi_pwm_led_write_raw,
};

#define AXI_PWM_LED_CHANNEL(num, addr) {				\
	.type = IIO_COUNT,						\
	.output = 1,							\
	.channel = num,							\
	.indexed = 1,							\
	.address = addr,						\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),			\
}

static const struct iio_chan_spec axi_pwm_led_channels[] = {
	AXI_PWM_LED_CHANNEL(0, AXI_PWM_LED_CHANNEL_0),
	AXI_PWM_LED_CHANNEL(1, AXI_PWM_LED_CHANNEL_1),
	AXI_PWM_LED_CHANNEL(2, AXI_PWM_LED_CHANNEL_2),
	AXI_PWM_LED_CHANNEL(3, AXI_PWM_LED_CHANNEL_3),
	AXI_PWM_LED_CHANNEL(4, AXI_PWM_LED_CHANNEL_4),
	AXI_PWM_LED_CHANNEL(5, AXI_PWM_LED_CHANNEL_5),
};

static int axi_pwm_led_probe(struct platform_device *pdev)
{
	struct iio_dev *indio_dev;
	struct axi_pwm_led_state *st;

	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->name = "axi-pwm-led";
	indio_dev->info = &axi_pwm_led_info;
	indio_dev->channels = axi_pwm_led_channels;
	indio_dev->num_channels = ARRAY_SIZE(axi_pwm_led_channels);

	st->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(st->regs))
		return PTR_ERR(st->regs);

	return devm_iio_device_register(&pdev->dev, indio_dev);
}

static const struct of_device_id axi_pwm_led_match[] = {
	{ .compatible = "adi,axi-pwm-led" },
	{}
};

static struct platform_driver one_bit_adc_dac_driver = {
	.driver = {
		.name = "axi-pwm-led",
		.of_match_table = axi_pwm_led_match,
	},
	.probe = axi_pwm_led_probe,
};

module_platform_driver(one_bit_adc_dac_driver);
MODULE_DEVICE_TABLE(of, one_bit_adc_dac_dt_match);

MODULE_AUTHOR("Ciprian Hegbeli <ciprian.hegbeli@analog.com>");
MODULE_DESCRIPTION("Analog Devices Axi PWM LED driver");
MODULE_LICENSE("GPL v2");
