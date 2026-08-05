#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,
			struct iio_chan_spec const *chan,
			int *val,
			int *val2,
			long mask)
{
    switch(mask){
        case IIO_CHAN_INFO_RAW:
            switch(chan->channel) {
                case 0:
                    *val = 10;
                    break;
                case 1:
                    *val = 11;
                    break;
                case 2:
                    *val = 12;
                    break;
                case 3:
                    *val = 13;
                    break;
                case 4:
                    *val = 14;
                    break;
                case 5:
                    *val = 15;
                    break;
                default:
                    return -EINVAL;
            }
            return IIO_VAL_INT;
        default:
            return -EINVAL;
    }
}

static int iio_adc_emu_write_raw(struct iio_dev *indio_dev,
			 struct iio_chan_spec const *chan,
			 int val,
			 int val2,
			 long mask)
{
    switch(mask) {
        case IIO_CHAN_INFO_RAW:
            switch(chan->channel) {
                case 0:
                    dev_info(&indio_dev->dev, "Trying to write to channel 0");
                    break;
                case 1:
                    dev_info(&indio_dev->dev, "Trying to write to channel 1");
                    break;
                case 2:
                    dev_info(&indio_dev->dev, "Trying to write to channel 2");
                    break;
                case 3:
                    dev_info(&indio_dev->dev, "Trying to write to channel 3");
                    break;
                case 4:
                    dev_info(&indio_dev->dev, "Trying to write to channel 4");
                    break;
                case 5:
                    dev_info(&indio_dev->dev, "Trying to write to channel 5");
                    break;
                default:
                    return -EINVAL;
            }
            return 0;
        default:
            return -EINVAL;
    }
}

static const struct iio_chan_spec iio_ad5592r_s_channels[] = {
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

static const struct iio_info ad5592r_s_info = {
    .read_raw = &iio_adc_emu_read_raw,
    .write_raw = &iio_adc_emu_write_raw
};

static int ad5592r_s_probe(struct spi_device *spi) {
    struct iio_dev *indio_dev;

    indio_dev = devm_iio_device_alloc(&spi->dev, 0);

    indio_dev->name = "ad5592r_s";
    indio_dev->info = &ad5592r_s_info;
    indio_dev->channels = iio_ad5592r_s_channels;
    indio_dev->num_channels = 6;

    return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
    .driver = {
        .name = "ad5592r_s"
    },
    .probe = ad5592r_s_probe
};

module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Berinde Codrin <codrin.berinde@gmail.com>");
MODULE_DESCRIPTION("IIO AD5592R_S");
MODULE_LICENSE("GPL v2");