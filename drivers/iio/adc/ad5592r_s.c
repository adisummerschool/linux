// SPDX-License-Identifier: GPL-2.0
/*
 * AD7170/AD7171 and AD7780/AD7781 SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 * Copyright 2019 Renato Lui Geh
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/unaligned.h>
#include <linux/bitfield.h>

const int CHANNEL_0 = 0;
const int CHANNEL_1 = 1;
const int CHANNEL_2 = 2;
const int CHANNEL_3 = 3;
const int CHANNEL_4 = 4;
const int CHANNEL_5 = 5;

#define AD5592R_S_MSB_MSK		    BIT(15)
#define AD5592R_ADDR_MSK		    GENMASK(14, 11)
#define AD5592R_DATA_MSK		    GENMASK(10, 0)

#define AD5592R_REG_RDB_ADDR      	0x07
#define AD5592R_REG_RD_EN_MSK		BIT(6)
#define AD5592R_REG_RD_ADDR_MSK		GENMASK(5, 2)

#define AD5592R_REG_PD_REF_CTRL		0x0B
#define AD5592R_POWER_ENABLE		0x00
#define AD5592R_POWER_DISABLE		BIT(10)
#define AD5592R_REG_EN_IREF		    BIT(9)

#define AD5592R_REG_SEQ           0X02


#define AD5592R_REG_ADC_CONFIG      0x04
#define AD5592R_REG_OUTPUT          GENMASK(11,0)

struct iio_adc_st {
	struct spi_device *spi;
    int reg_select;
    int chan_val[6];
};

static int iio_ad5592r_s_spi_write(struct iio_adc_st *st, u8 addr, u16 data){

    u16 tx = 0;
    u16 package = 0;
    struct spi_transfer t = {
            .tx_buf = &package,
            .len = 2
    };

    tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) | FIELD_PREP(AD5592R_ADDR_MSK, addr) | FIELD_PREP(AD5592R_DATA_MSK, data);
    put_unaligned_be16(tx, &package);

    dev_info(&st->spi->dev, "tx we constructed %x\n", tx);
    dev_info(&st->spi->dev, "package we constructed %x\n", package);

    return spi_sync_transfer(st->spi, &t, 1);
}

static int iio_ad5592r_s_spi_read(struct iio_adc_st *st, u8 addr, u16 *data){
  
    u16 rx = 0;
	u16 reg_rdb_data = 0;
	u16 received_data = 0;
    int ret = 0;
		
    struct spi_transfer t = {
			.tx_buf = NULL,
            .rx_buf = &rx,
            .len = 2
    };

	reg_rdb_data = FIELD_PREP(AD5592R_REG_RD_EN_MSK, 1) | FIELD_PREP(AD5592R_REG_RD_ADDR_MSK, addr);

	ret = iio_ad5592r_s_spi_write(st, AD5592R_REG_RDB_ADDR, reg_rdb_data);
    if(ret){
        dev_info(&st->spi->dev, "Writing the readback register failed%d\n", ret);
        return ret;
    }

	ret = spi_sync_transfer(st->spi, &t, 1);
	if(ret){
		dev_info(&st->spi->dev, "Failed receiving readback%d\n", ret);
		return ret;
	}

	received_data = get_unaligned_be16(&rx);
    *data = FIELD_GET(AD5592R_DATA_MSK, received_data);
    return 0;
}

static int iio_ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev, unsigned reg, unsigned writeval, unsigned *readval){
    
    struct iio_adc_st *st = iio_priv(indio_dev);
    
    if(readval) 
        return iio_ad5592r_s_spi_read(st,reg, (u16 *)readval);
    return iio_ad5592r_s_spi_write(st,reg, writeval);
}

static int iio_ad5592r_s_read_chan(struct iio_adc_st *st, int channel, u16 *readval){
    
    u16 data = 0;
    u16 packet = 0;
    
    int ret = 0;

    ret = iio_ad5592r_s_spi_write(st, AD5592R_REG_SEQ, BIT(channel));
    if(ret){
        dev_err(&st->spi->dev, "Writing conversion reg failed: %d\n", ret);
        return ret;
    }

    struct spi_transfer t ={
        .tx_buf = NULL,
        .rx_buf = &packet,
        .len = 2
    };

    ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret) {
		dev_err(&st->spi->dev, "Failed receiving readback %d\n", ret);
		return ret;
	}

    ret = spi_sync_transfer(st->spi, &t, 1);
	if (ret) {
		dev_err(&st->spi->dev, "Failed receiving readback %d\n", ret);
		return ret;
	}
    data = get_unaligned_be16(&packet);
    *readval = FIELD_GET(AD5592R_REG_OUTPUT,data);
    return 0;
}

static int iio_ad5592r_s_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
{
    struct iio_adc_st *st = iio_priv(indio_dev);
    int ret;

    switch(mask){
        case IIO_CHAN_INFO_RAW:               
            if (!st->reg_select) {
                ret = iio_ad5592r_s_read_chan(st, chan->channel, (u16 *)val);
                if(ret){
                    dev_err(&st->spi->dev, "Reading from channels failed: %d\n", ret);
                    return ret;
                }
                return IIO_VAL_INT; 
            }
            else 
                return -EINVAL;               
        case IIO_CHAN_INFO_ENABLE:
            *val = st->reg_select;
            return IIO_VAL_INT;
        default: 
            return -EINVAL;
    }
}

static int iio_ad5592r_s_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int val, int val2, long mask)
{
    struct iio_adc_st *st = iio_priv(indio_dev);
    switch(mask){
        case IIO_CHAN_INFO_RAW:                
            if(!st->reg_select){
                if(chan->channel == CHANNEL_0){
                    dev_info(&indio_dev->dev, "Trying to write to channel 0: %d", val);
                    //st->chan_val[0] = val;
                }
                else  if(chan->channel == CHANNEL_1){
                        dev_info(&indio_dev->dev, "Trying to write to channel 1: %d", val);
                        //st->chan_val[1] = val;
                    }
                    else  if(chan->channel == CHANNEL_2){
                            dev_info(&indio_dev->dev, "Trying to write to channel 2: %d", val); 
                            //st->chan_val[2] = val; 
                    }
                        else if(chan->channel == CHANNEL_3){
                                dev_info(&indio_dev->dev, "Trying to write to channel 3: %d", val);
                                //st->chan_val[3] = val;
                        }
                            else if(chan->channel == CHANNEL_4){
                                    dev_info(&indio_dev->dev, "Trying to write to channel 4: %d", val); 
                                    //st->chan_val[4] = val;
                            }
                                else if(chan->channel == CHANNEL_5){
                                        dev_info(&indio_dev->dev, "Trying to write to channel 5: %d", val); 
                                        //st->chan_val[5] = val;
                                }
                return 0;
            }
            else
                return -EINVAL;  
        case IIO_CHAN_INFO_ENABLE:
            st->reg_select = val ? 1 : 0;
            return 0;                  
        default: 
            return -EINVAL;
    }
}
static const struct iio_chan_spec iio_ad5592r_s_channels[] = {
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_0,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_1,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_2,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_3,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_4,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    },
    {
        .type = IIO_VOLTAGE,
        .channel = CHANNEL_5,
        .indexed = 1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
    }
};


static const struct iio_info iio_ad5592r_s_info = {
    .read_raw = &iio_ad5592r_s_read_raw,
    .write_raw = &iio_ad5592r_s_write_raw,
	.debugfs_reg_access = &iio_ad5592r_s_debugfs_reg_access
};

// probe function
static int iio_ad5592r_s_probe(struct spi_device *spi){
    struct iio_dev *indio_dev;
    indio_dev = devm_iio_device_alloc(&spi->dev,0);

    struct iio_adc_st *st;
    indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
    st = iio_priv(indio_dev);
    st->reg_select = 1;
	st->spi = spi;
    memset(st->chan_val, 0, sizeof(st->chan_val));

	iio_ad5592r_s_spi_write(st, AD5592R_REG_PD_REF_CTRL, FIELD_PREP(AD5592R_REG_EN_IREF,1));
    iio_ad5592r_s_spi_write(st, AD5592R_REG_ADC_CONFIG, GENMASK(5,0));

    indio_dev->name = "ad5592r_s";
    indio_dev->info = &iio_ad5592r_s_info;
    indio_dev->channels = iio_ad5592r_s_channels;
    indio_dev->num_channels = 6;
    indio_dev->num_channels = ARRAY_SIZE(iio_ad5592r_s_channels);

    return devm_iio_device_register(&spi->dev, indio_dev);
} 


static struct spi_driver iio_ad5592r_s_driver = {
    .driver = {
        .name = "ad5592r_s"
    },
    .probe = iio_ad5592r_s_probe
};
module_spi_driver(iio_ad5592r_s_driver);


MODULE_AUTHOR("Mascasan Maya-Lorena <mascasan.se.maya@student.utcluj.ro>");
MODULE_DESCRIPTION("Analog Devices - ADC Driver");
MODULE_LICENSE("GPL v2");