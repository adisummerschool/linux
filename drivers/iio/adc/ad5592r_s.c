/* SPDX-License-Identifier: GPL-2.0 */
/*
 * AD5592R SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 */

 #include <linux/unaligned.h>
 #include <linux/bitfield.h>
 #include <linux/module.h>
 #include <linux/unaligned.h>

 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>
 #include <linux/iio/triggered_buffer.h>
 #include <linux/iio/trigger_consumer.h>

 #define ADC_AD5592R_S_MSB_MSK   		BIT(15)
 #define ADC_AD5592R_S_ADDR_MSK   		GENMASK(14,11)
 #define ADC_AD5592R_S_WR_DATA_MSK   	GENMASK(10,0)

 #define ADC_AD5592R_S_EN_READB			BIT(6)
 #define ADC_AD5592R_S_REG_RDB_ADDR		0X7
 #define ADC_AD5592R_S_SELECT_RDB		GENMASK(5,2)

 #define ADC_AD5592R_S_REG_PD_ADDR		0xB	
 #define ADC_AD5592R_S_REG_EN_IREF		BIT(9)	

 #define ADC_AD5592R_S_ADC_REG          0x04
 #define ADC_AD5592R_S_ADC_ENABLE		GENMASK(5,0)

 #define ADC_AD5592R_S_REG_SEQ_ADDR     0x02
 #define ADC_AD5592R_S_REG_SEQ_MSK      GENMASK(5,0)
 #define ADC_AD5592R_S_REG_SEQ_CHAN(x)		BIT(x)

 #define ADC_AD5592R_S_CONV_DATA        GENMASK(11,0)

 #define ADC_AD5592R_S_REG_CFG_MSK		GENMASK(5,0)			


struct adc_ad5592r_s_st {
	struct spi_device *spi;
	bool reg_select;
	int chan_val[6];
};


 static int adc_ad5592r_s_spi_write(struct adc_ad5592r_s_st *st, u8 addr, u16 data)
 {
        u16 tx = 0;
        u16 package =0;
        struct spi_transfer t = {
                .tx_buf = &package,
                .len = 2
                
        };

        tx = FIELD_PREP(ADC_AD5592R_S_MSB_MSK, 0) | 
			 FIELD_PREP(ADC_AD5592R_S_ADDR_MSK, addr) | 
			 FIELD_PREP(ADC_AD5592R_S_WR_DATA_MSK, data);
			 
        put_unaligned_be16(tx, &package);

        // dev_info(&st->spi->dev, "spi write msg tx %x\n", tx);
        // dev_info(&st->spi->dev, "spi write msg package %x\n", package);


        return spi_sync_transfer(st->spi, &t, 1);


 }

 static int adc_ad5592r_s_spi_read(struct adc_ad5592r_s_st *st, u8 addr, u16 *data)
 {
        u16 rx = 0;
		u16 reg_rdb_data;
        int ret = 0;
		u16 rcv_data;
        struct spi_transfer t[]={
                { 
						.tx_buf = NULL,
						.rx_buf = &rx,
                        .len = 2
                }
        };

		reg_rdb_data = FIELD_PREP(ADC_AD5592R_S_EN_READB, 1) |
					   FIELD_PREP(ADC_AD5592R_S_SELECT_RDB, addr);
		ret = adc_ad5592r_s_spi_write(st, ADC_AD5592R_S_REG_RDB_ADDR, reg_rdb_data);

        if(ret) {
                dev_info(&st->spi->dev, "Writing the readback register failed %d\n", ret);
                return ret;
        }

		ret = spi_sync_transfer(st->spi, t, 1);
		if(ret) {
                dev_info(&st->spi->dev, "Failed recieveing readback %d\n", ret);
                return ret;
        }
		rcv_data = get_unaligned_be16(&rx);
        *data = FIELD_GET(ADC_AD5592R_S_WR_DATA_MSK, rcv_data);
        return 0;
 }

 static int adc_ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev,
                                        unsigned reg,unsigned writeval,
                                        unsigned *readval)
 {
        struct adc_ad5592r_s_st *st = iio_priv(indio_dev);
        
        if (readval)
               return adc_ad5592r_s_spi_read(st, reg, (u16 *) readval);

        return adc_ad5592r_s_spi_write(st, reg, writeval);

 }

 static int adc_ad5592r_s_read_chan(struct adc_ad5592r_s_st *st, 
                                int channel,
                                u16*readval)
{
		u16 rx = 0;
        u16 adc_data = 0;
        int ret = 0;

		struct spi_transfer t = {
			.rx_buf = &rx,
			.len = 2,
		};

        ret = adc_ad5592r_s_spi_write(st, ADC_AD5592R_S_REG_SEQ_ADDR,
                              ADC_AD5592R_S_REG_SEQ_CHAN(channel));
        if (ret) {
                dev_err(&st->spi->dev, "ADC SEQ selection failed: %d\n", ret);
                return ret;
        }


        ret = spi_sync_transfer(st->spi, &t, 1);
        if (ret) {
                dev_err(&st->spi->dev, "ADC dummy transfer failed: %d\n", ret);
                return ret;
        }

        ret = spi_sync_transfer(st->spi, &t, 1);
        if (ret) {
                dev_err(&st->spi->dev, "ADC conversion read failed: %d\n", ret);
                return ret;
        }

        adc_data = get_unaligned_be16(&rx);
        *readval = FIELD_GET(ADC_AD5592R_S_CONV_DATA,adc_data);
        return 0;
}

static int adc_ad5592r_s_read_raw(struct iio_dev *indio_dev,
				  struct iio_chan_spec const *chan, int *val,
				  int *val2, long mask)
{
	struct adc_ad5592r_s_st *st = iio_priv(indio_dev);

	int ret = 0;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			ret = adc_ad5592r_s_read_chan(st, chan->channel, (u16 *) val);
			if (ret) {
                dev_err(&st->spi->dev, "ADC conversion READ failed: %d\n", ret);
                return ret;
        	}
			return IIO_VAL_INT;
		} else {
			return -EINVAL;
		}
	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int adc_ad5592r_s_write_raw(struct iio_dev *indio_dev,
				   struct iio_chan_spec const *chan, int val,
				   int val2, long mask)
{
	struct adc_ad5592r_s_st *st = iio_priv(indio_dev);
	
	switch (mask) {
	case IIO_CHAN_INFO_RAW:

		if (!st->reg_select) {
			switch (chan->channel) {
			case 0:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 0");
				//st->chan_val[0] = val;
				break;
			case 1:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 1");
				//st->chan_val[1] = val;
				break;
			case 2:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 2");
				//st->chan_val[2] = val;
				break;
			case 3:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 3");
				//st->chan_val[3] = val;
				break;
			case 4:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 4");
				//st->chan_val[4] = val;
				break;
			case 5:
				dev_info(&indio_dev->dev,
					 "Trying to write to channel 5");
				//st->chan_val[5] = val;
				break;
			default:
				dev_info(&indio_dev->dev,
					 "Channel 7 and 8 are not accessible.");
				return -EINVAL;
			}
			return 0;
		} else {
			return -EINVAL;
		}

	case IIO_CHAN_INFO_ENABLE:
		st->reg_select = val ? 1 : 0;
		return 0;

	default:
		return -EINVAL;
	}
}

static irqreturn_t adc_ad5592r_s_trigger_handler(int irq, void *p)
 {
        struct iio_poll_func *pf = p;
        struct iio_dev *indio_dev = pf->indio_dev;
        struct adc_ad5592r_s_st *st = iio_priv(indio_dev);
        int bit = 0;
        int ret;
        u16 buf[6];
        int i = 0;

        
        for_each_set_bit(bit, indio_dev->active_scan_mask, indio_dev->num_channels)
        {
                ret = adc_ad5592r_s_read_chan(st, bit, &buf[i++]);
                if (ret) {
                        dev_err(&indio_dev->dev, "Reading chan failed in buffer, err: %d\n", ret);
                        iio_trigger_notify_done(indio_dev->trig);
                        return IRQ_HANDLED;
                }
        }
        iio_push_to_buffers(indio_dev, buf);
        iio_trigger_notify_done(indio_dev->trig);
                        return IRQ_HANDLED;
 }

static const struct iio_chan_spec adc_ad5592r_s_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 0,
      	.scan_type = {
                .sign = 'u',
                .realbits = 12,
                .storagebits = 16
		}
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 1,
      	.scan_type = {
                .sign = 'u',
                .realbits = 12,
                .storagebits = 16
		}
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 2,
      	.scan_type = {
                .sign = 'u',
                .realbits = 12,
                .storagebits = 16
		}
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 3,
      	.scan_type = {
                .sign = 'u',
                .realbits = 12,
                .storagebits = 16
		}
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 4,
      	.scan_type = {
                .sign = 'u',
                .realbits = 12,
                .storagebits = 16
		}
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 5,
      	.scan_type = {
                .sign = 'u',
                .realbits = 12,
                .storagebits = 16
		}
	}
};

static const struct iio_info ad5592r_s_info = {
	.read_raw = &adc_ad5592r_s_read_raw,
	.write_raw = &adc_ad5592r_s_write_raw,
	.debugfs_reg_access = &adc_ad5592r_s_debugfs_reg_access
};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct adc_ad5592r_s_st *st;
	int ret;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->reg_select = 1;
	st->spi = spi;
	memset(st->chan_val, 0, sizeof(sizeof(st->chan_val)));

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = adc_ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(adc_ad5592r_s_channels);

	adc_ad5592r_s_spi_write(st, ADC_AD5592R_S_REG_PD_ADDR, 
						FIELD_PREP(ADC_AD5592R_S_REG_EN_IREF, 1));
	
	adc_ad5592r_s_spi_write(st, ADC_AD5592R_S_ADC_REG, 
						FIELD_PREP(ADC_AD5592R_S_REG_CFG_MSK,
							ADC_AD5592R_S_ADC_ENABLE));

	ret = devm_iio_triggered_buffer_setup(&spi->dev, indio_dev, NULL, 
                                &adc_ad5592r_s_trigger_handler, NULL);
    if (ret) {
        dev_err(&spi->dev, "Failed to creat buffer");
        return ret;
    }

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
    .driver = {
        .name = "ad5592r_s",
    },
    .probe = ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Dradici Leon");
MODULE_DESCRIPTION("Analog Devices AD5592R ADC Summer School");
MODULE_LICENSE("GPL v2");