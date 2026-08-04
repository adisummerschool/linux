#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

static const struct iio_info ad5592rs_info = {
};

static int ad5592rs_probe(struct spi_device *spi)
{
    struct iio_dev *indio_dev;
    indio_dev = devm_iio_device_alloc(&spi->dev, 0);

    indio_dev->name = "ad5592r_s";
    indio_dev->info = &ad5592rs_info;
    return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592rs_driver = {
    .driver = {
        .name = "ad5592r_s",
    },
    .probe = ad5592rs_probe,
};
module_spi_driver(ad5592rs_driver);

MODULE_AUTHOR("Denisa Dersedan <denisa.dersedan@analog.com>");
MODULE_DESCRIPTION("AD5592R Driver");
MODULE_LICENSE("GPL v2");