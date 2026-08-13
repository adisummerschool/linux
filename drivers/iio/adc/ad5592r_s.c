// SPDX-License-Identifier: GPL-2.0
/*
 * IIO SPI ADC driver for AD5592R - Summer Practice
 * Copyright 2011 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/unaligned.h>
#include <linux/bitfield.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>

/* Datasheet Table 10 & 13: Register Addresses and Masks */
#define AD5592R_S_MSB_MSK       BIT(15)
#define AD5592R_S_ADDR_MSK      GENMASK(14,11)
#define AD5592R_S_DATA_MSK      GENMASK(10,0)

#define AD5592R_REG_NOP             0x00
#define AD5592R_REG_ADC_SEQ         0x02
#define AD5592R_REG_ADC_CONFIG      0x04
#define AD5592R_REG_READ_AND_LDAC   0x07
#define AD5592R_REG_PD_REF_CTRL     0x0B

#define AD5592R_ENBL_RDBK           BIT(6) // Bit 6 in Reg 0x07 enables readback
#define AD5592R_REG_SLCT_RDBK       GENMASK(5,2) // Bits 5:2 in Reg 0x07 select register
#define AD5592R_EN_REF_MSK          BIT(9) // Bit 9 in Reg 0x0B enables internal reference
#define AD5592R_PD_ALL_MSK          BIT(10)// Bit 10 in Reg 0x0B powers down the chip
#define AD5592R_ADC_DATA_MSK        GENMASK(11,0) // 12-bit ADC result mask

struct iio_adc_st_prv 
    {
        int reg_select;          
        struct spi_device *spi;
        
        __be16 tx_buf;
        __be16 rx_buf;
    };

static int ad5592r_s_spi_write(struct iio_adc_st_prv *st, u8 addr, u16 data)
    {
        u16 tx = 0;
        struct spi_transfer t = {
            .tx_buf = &st->tx_buf,
            .len = 2
        };
        tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) | FIELD_PREP(AD5592R_S_ADDR_MSK, addr) | FIELD_PREP(AD5592R_S_DATA_MSK, data);   
        put_unaligned_be16(tx, &st->tx_buf);
        return spi_sync_transfer(st->spi, &t, 1);
    }

static int ad5592r_s_spi_read(struct iio_adc_st_prv *st, u8 addr, u16 *data)
    {
        u16 reg_db_data;
        u16 rcv_data;
        int ret = 0;
        
        struct spi_transfer t = {
            .tx_buf = &st->tx_buf,
            .rx_buf = &st->rx_buf,
            .len = 2
        };
        
        reg_db_data = FIELD_PREP(AD5592R_ENBL_RDBK, 1) | FIELD_PREP(AD5592R_REG_SLCT_RDBK, addr);
        ret = ad5592r_s_spi_write(st, AD5592R_REG_READ_AND_LDAC, reg_db_data);
        if(ret) return ret;

        put_unaligned_be16(0x0000, &st->tx_buf); 
        ret = spi_sync_transfer(st->spi, &t, 1);   
        if(ret) return ret;
        
        rcv_data = get_unaligned_be16(&st->rx_buf);
        *data = FIELD_GET(AD5592R_S_DATA_MSK, rcv_data);
        
        return 0;
    }

static int ad5592r_read_adc_channel(struct iio_adc_st_prv *st, int channel, u16 *readval)
    {
        int ret;
        struct spi_transfer t = {
            .tx_buf = &st->tx_buf,
            .rx_buf = &st->rx_buf,
            .len = 2
        };
        
        ret = ad5592r_s_spi_write(st, AD5592R_REG_ADC_SEQ, BIT(channel));
        if (ret) return ret;

        put_unaligned_be16(0x0000, &st->tx_buf);
        ret = spi_sync_transfer(st->spi, &t, 1);
        if (ret) return ret;

        ret = spi_sync_transfer(st->spi, &t, 1);
        if (ret) return ret;

        *readval = get_unaligned_be16(&st->rx_buf) & AD5592R_ADC_DATA_MSK;

        return 0;
    }

static int ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev, unsigned reg, unsigned writeval, unsigned *readval)
    {
        struct iio_adc_st_prv *st = iio_priv(indio_dev);
        if (readval)
            return ad5592r_s_spi_read(st, reg, (u16 *)readval);
        return ad5592r_s_spi_write(st, reg, (u16)writeval);
    }

static int iio_adc_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
    {
        struct iio_adc_st_prv *st = iio_priv(indio_dev);
        u16 adc_data = 0;
        int ret;
        switch (mask) 
            {
                case IIO_CHAN_INFO_RAW:
                    if (st->reg_select) 
                        {
                            ret = ad5592r_read_adc_channel(st, chan->channel, &adc_data);
                            if (ret)
                                return ret;
                            
                            *val = adc_data;
                            return IIO_VAL_INT;   
                        } 
                    else 
                        {
                            return -EINVAL; // Device is disabled
                        }
                    
                case IIO_CHAN_INFO_ENABLE:
                    *val = st->reg_select;
                    return IIO_VAL_INT;
                    
                default:
                    return -EINVAL;
            }
    }
 
static int iio_adc_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int val, int val2, long mask)
    {
        struct iio_adc_st_prv *st = iio_priv(indio_dev);
        switch(mask) 
            {
                case IIO_CHAN_INFO_RAW:
                    return -EINVAL; 
                    
                case IIO_CHAN_INFO_ENABLE:
                    st->reg_select = val ? 1 : 0;
                    if (st->reg_select) 
                        {
                            ad5592r_s_spi_write(st, AD5592R_REG_PD_REF_CTRL, AD5592R_EN_REF_MSK);
                        } 
                    else 
                        {
                            ad5592r_s_spi_write(st, AD5592R_REG_PD_REF_CTRL, AD5592R_PD_ALL_MSK);
                        }
                    return 0; 

                default:
                    return -EINVAL;     
            }    
    }

static irqreturn_t iio_adc_trigger_handler(int irq, void *p)
{
    struct iio_poll_func *pf = p;
    struct iio_dev *indio_dev = pf->indio_dev;
    struct iio_adc_st_prv *st = iio_priv(indio_dev);
    int bit, ret;
    int i = 0;
    /* Struct ensures data is perfectly aligned in memory for the IIO buffer */
    struct 
        {
            u16 values[6]; // We have 6 channels maximum
            s64 timestamp __aligned(8);
        } scan;
    memset(&scan, 0, sizeof(scan));
    /* Loop through only the channels the user enabled for the buffer */
    for_each_set_bit(bit, indio_dev->active_scan_mask, indio_dev->num_channels) 
        {
            // Reuse our helper function to get the data!
            ret = ad5592r_read_adc_channel(st, bit, &scan.values[i]);
            if (ret) 
                {
                    dev_err(&st->spi->dev, "Buffer read failed for channel %d\n", bit);
                    goto done;
                }
            i++;
        }

    /* Push the aligned struct to the IIO buffer */
    iio_push_to_buffers_with_timestamp(indio_dev, &scan, iio_get_time_ns(indio_dev)); 

done:
    iio_trigger_notify_done(indio_dev->trig);
    return IRQ_HANDLED;   
}

/* 
 * MACRO to make adding channels clean and prevent copy-paste errors
 * Notice that scan_index is mapped perfectly to avoid the buffer bug!
 */
#define AD5592R_ADC_CHANNEL(_channel) { \
    .type = IIO_VOLTAGE, \
    .channel = (_channel), \
    .indexed = 1, \
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW), \
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE), \
    .scan_index = (_channel), \
    .scan_type = { \
        .sign = 'u', \
        .realbits = 12, \
        .storagebits = 16, \
        .endianness = IIO_CPU, \
    } \
}

static const struct iio_chan_spec iio_adc_channels[] = {
    AD5592R_ADC_CHANNEL(0),
    AD5592R_ADC_CHANNEL(1),
    AD5592R_ADC_CHANNEL(2),
    AD5592R_ADC_CHANNEL(3),
    AD5592R_ADC_CHANNEL(4),
    AD5592R_ADC_CHANNEL(5)
};

static const struct iio_info adc_ad5592_driver_info = {
    .read_raw = &iio_adc_read_raw,
    .write_raw = &iio_adc_write_raw,
    .debugfs_reg_access = &ad5592r_s_debugfs_reg_access
};

static int ad5592r_s_probe(struct spi_device *spi)
    {
        struct iio_dev *indio_dev;
        struct iio_adc_st_prv *st;
        int ret;
        
        indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
        if (!indio_dev)
            return -ENOMEM;

        st = iio_priv(indio_dev);
        st->reg_select = 1; // Enabled by default
        st->spi = spi;

        // 1. Configure pins 0-5 as ADC inputs
        ad5592r_s_spi_write(st, AD5592R_REG_ADC_CONFIG, GENMASK(5,0));
        // 2. Power on reference
        ad5592r_s_spi_write(st, AD5592R_REG_PD_REF_CTRL, AD5592R_EN_REF_MSK);

        indio_dev->name = "ad5592r_s";
        indio_dev->info = &adc_ad5592_driver_info;
        indio_dev->channels = iio_adc_channels;
        indio_dev->num_channels = ARRAY_SIZE(iio_adc_channels);

        // Setup the triggered buffer!
        ret = devm_iio_triggered_buffer_setup(&spi->dev, indio_dev, NULL, &iio_adc_trigger_handler, NULL);
        if(ret) 
            {
                dev_err(&spi->dev, "Failed to create buffer\n");
                return ret;
            }

        return devm_iio_device_register(&spi->dev, indio_dev);
    }

static struct spi_driver ad5592r_s_driver = {
    .driver = {
        .name = "ad5592r_s"
    },
    .probe = ad5592r_s_probe
};

module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Molnar Levente <mlevente388@gmail.com>");
MODULE_DESCRIPTION("IIO setup for AD5592 - summer practice setup");
MODULE_LICENSE("GPL v2");