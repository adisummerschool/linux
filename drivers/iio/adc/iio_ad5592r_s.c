#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/unaligned.h>
#include <linux/bitfield.h>

#define AD5592R_S_MSB_MSK	BIT(15)
#define AD5592R_S_ADDR_MSK	GENMASK(14,11)
#define AD5592R_S_DATA_MSK	GENMASK(10,0)

#define AD5592R_S_REG_RDB_ADDR	0x7
#define AD5592R_S_EN_READB	BIT(6)
#define AD5592R_S_REG_SELECT	GENMASK(5,2)

#define AD5592R_S_REG_PD_ADDR	0xB
#define AD5592R_S_REG_EN_IREF	BIT(9)

struct iio_ad5592r_s_st {
	int en;
	struct spi_device *spi;
	int reg_select;
	int chan_val[6];
};

static int ad5592r_s_spi_write(struct iio_ad5592r_s_st *st, u8 addr, u16 data)
{
	u16 tx = 0;
	u16 package = 0;
	struct spi_transfer t = {
		.tx_buf = &package,
		.len = 2
	};

	tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) | 
	     FIELD_PREP(AD5592R_S_ADDR_MSK, addr) |
	     FIELD_PREP(AD5592R_S_DATA_MSK, data);

	put_unaligned_be16(tx, &package);

	dev_info(&st->spi->dev, "SPI WRITE msg tx %x\n", tx);
	dev_info(&st->spi->dev, "SPI WRITE msg package %x\n", package);

	return spi_sync_transfer(st->spi, &t, 1);
}

static int ad5592r_s_spi_read(struct iio_ad5592r_s_st *st, u8 addr, u16 *data)
{
	u16 rx = 0;
	u16 reg_rdb_data;
	u16 rcv_data = 0;
	int ret = 0;
	struct spi_transfer t ={
		.tx_buf = NULL,
		.rx_buf = &rx,
		.len = 2
	};
	reg_rdb_data = FIELD_PREP(AD5592R_S_EN_READB, 1) | 
		       FIELD_PREP(AD5592R_S_REG_SELECT, addr);
	ret = ad5592r_s_spi_write(st, AD5592R_S_REG_RDB_ADDR, reg_rdb_data);
	
	if(ret){
		dev_info(&st->spi->dev, "Writing the read register failed");
		return ret;
	}
	ret = spi_sync_transfer(st->spi, &t, 1);
	if(ret){
		dev_info(&st->spi->dev, "Failed receiveing readback");
		return ret;
	}

	rcv_data = get_unaligned_be16(&rx);
	*data = FIELD_GET(AD5592R_S_DATA_MSK, rcv_data);
	return 0;
}

static int ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev,
				unsigned reg, unsigned writeval,
				unsigned *readval)
{
	struct iio_ad5592r_s_st *st = iio_priv(indio_dev);
	if (readval) {
		return ad5592r_s_spi_read(st, reg, (u16 *)readval);
	}
	return ad5592r_s_spi_write(st, reg, writeval);
} ///unfinished





static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan, int *val,
				int *val2, long mask)
{
	struct iio_ad5592r_s_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			switch (chan->channel) {
			case 0:
				*val = st->chan_val[0];
				break;
			case 1:
				*val = st->chan_val[1];
				break;
			case 2:
				*val = st->chan_val[2];
				break;
			case 3:
				*val = st->chan_val[3];
				break;
			case 4:
				*val = st->chan_val[4];
				break;
			case 5:
				*val = st->chan_val[5];
				break;
			default:
				return -EINVAL;
			}
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
	struct iio_ad5592r_s_st *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			switch (chan->channel) {
			case 0:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 1");
				st->chan_val[0] = val;
				break;
			case 1:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 2");
				st->chan_val[1] = val;
				break;
			case 2:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 3");
				st->chan_val[2] = val;
				break;
			case 3:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 4");
				st->chan_val[3] = val;
				break;
			case 4:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 5");
				st->chan_val[4] = val;
				break;
			case 5:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 6");
				st->chan_val[5] = val;
				break;
			default:
				return -EINVAL;
			}
			return 0;
		} else
			return -EINVAL;

	case IIO_CHAN_INFO_ENABLE:
		st->reg_select = val ? 1 : 0;
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct iio_chan_spec iio_ad5592r_s_channels[] = {
	{ .type = IIO_VOLTAGE,
	  .channel = 0,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 1,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 2,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 3,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 4,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
	{ .type = IIO_VOLTAGE,
	  .channel = 5,
	  .indexed = 1,
	  .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	  .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE) },
};

static const struct iio_info iio_ad5592r_s_info = {
	.read_raw = &iio_adc_emu_read_raw,
	.write_raw = &iio_adc_emu_write_raw,
	.debugfs_reg_access = &ad5592r_s_debugfs_reg_access
};

static int iio_ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_ad5592r_s_st *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->spi = spi;
	st->reg_select = 1; //active low, 1 = off
	memset(st->chan_val, 0, sizeof(st->chan_val)); //initialises chan to 0
	indio_dev->name = "iio_ad5592r_s";
	indio_dev->info = &iio_ad5592r_s_info;
	indio_dev->channels = iio_ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(iio_ad5592r_s_channels);

	ad5592r_s_spi_write(st, AD5592R_S_REG_PD_ADDR, 
			    FIELD_PREP(AD5592R_S_REG_EN_IREF, 1));

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver iio_ad5592r_s_driver = {
	.driver = { .name = "iio_ad5592r_s" },
	.probe = iio_ad5592r_s_probe
};

module_spi_driver(iio_ad5592r_s_driver);

MODULE_AUTHOR("Klampfl Helmut");
MODULE_DESCRIPTION("Analog Devices IIO ad5592r_s Summer School");
MODULE_LICENSE("GPL");