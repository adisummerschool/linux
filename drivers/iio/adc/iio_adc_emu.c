// SPDX-License-Identifier: GPL-2.0-only
/*
 * AD7785/AD7792/AD7793/AD7794/AD7795 SPI ADC driver
 *
 * Copyright 2011-2012 Analog Devices Inc.
 */

 #include <linux/unaligned.h>
 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>
 #include <linux/bitfield.h>

 #define EMU_RDWR_MSK BIT(7)
 #define EMU_ADDR_MASK GENMASK(14, 8)
 #define EMU_DATA_MASK GENMASK(7, 0)

 #define EMU_POWER_REG 0x02
 #define EMU_POWER_ENABLE 0x0
 #define EMU_POWER_DISABLE 0x20

 struct iio_adc_emu_st {
     struct spi_device *spi;
     int reg_select;
     int chan_val[2];
 };

 static int iio_adc_emu_spi_read(struct iio_adc_emu_st *st, u8 addr, u8 *data)
 {
   u8 tx = 0;
   u8 rx = 0;
   int ret = 0;
   struct spi_transfer t[] = {
   {
      .tx_buf = &tx,
      .len = 1
   },
   {
      .rx_buf = &rx,
      .len = 1
   }
   };

   tx = FIELD_PREP(EMU_RDWR_MSK, 1) | addr;
   ret = spi_sync_transfer(st->spi, t, 2);
   if(ret){
      dev_info(&st->spi->dev, "Spi READ transfer failed %d\n", ret);
      return ret;
   }
    *data = rx;
    return 0;

 }

 static int iio_adc_emu_spi_write(struct iio_adc_emu_st *st, u8 addr, u8 data)
 {
   u16 tx = 0;
   u16 package = 0;
   struct spi_transfer t = {
      .tx_buf = &package,
      .len = 2
   };

   tx = FIELD_PREP(EMU_RDWR_MSK, 0) | FIELD_PREP(EMU_ADDR_MASK, addr)
        | FIELD_PREP(EMU_DATA_MASK, data);
        put_unaligned_be16(tx, &package);

        dev_info(&st->spi->dev, "tx we constructed %x\n", tx);
        dev_info(&st->spi->dev, "package we constructed %x\n", package);

        return spi_sync_transfer(st->spi, &t, 1);
 }

 static int iio_adc_emu_debugfs_reg_access(struct iio_dev *indio_dev,
                                    unsigned reg, unsigned writeval,
                                    unsigned *readval)
  {
   struct iio_adc_emu_st *st = iio_priv(indio_dev);

   if(readval) {
     return iio_adc_emu_spi_read(st, reg, (u8 *) readval);
   }
   return iio_adc_emu_spi_write(st, reg, writeval);
  }                                  

 static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,
                               struct iio_chan_spec const *chan,
                               int *val,
                               int *val2,
                               long mask)
  {
   struct iio_adc_emu_st *st = iio_priv(indio_dev);
   switch (mask){
      case IIO_CHAN_INFO_RAW:
              if(!st->reg_select){
              if(chan->channel){
               *val = st->chan_val[1];
               return IIO_VAL_INT;
              }
              else {
               *val = st->chan_val[0];
               return IIO_VAL_INT;
              }
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
  
  static int iio_adc_emu_write_raw(struct iio_dev *indio_dev,
                              struct iio_chan_spec const *chan,
                              int val,
                              int val2,
                              long mask)
  {
   struct iio_adc_emu_st *st = iio_priv(indio_dev);
   switch (mask){
      case IIO_CHAN_INFO_RAW:
               if(!st->reg_select){
              if(chan->channel){
               dev_info(&indio_dev->dev, "Trying to write to channel 1");
               st->chan_val[1] = val;
              }
              else{
               dev_info(&indio_dev->dev, "Trying to write to channel 0");
               st->chan_val[0] = val;
              }
              return 0;
               }
               else 
               return -EINVAL;
      case IIO_CHAN_INFO_ENABLE:
           if(val){
             st->reg_select = 1;
             iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_DISABLE);
           }
           else {
             st->reg_select = 0;
             iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_ENABLE);
           }
             return 0;      
      default:
             return -EINVAL;
   }
  }                               
                              

 static const struct iio_chan_spec iio_adc_emu_channels[] = {
   {
   .type = IIO_VOLTAGE,
   .channel = 0,
   .indexed = 1,
   .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
   .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
 },
  {
   .type = IIO_VOLTAGE,
   .channel = 1,
   .indexed = 1,
   .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
   .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE)
 }

};
 static const struct iio_info iio_adc_emu_info = {
        .read_raw = &iio_adc_emu_read_raw,
        .write_raw = &iio_adc_emu_write_raw,
        .debugfs_reg_access = &iio_adc_emu_debugfs_reg_access

 };

 static int iio_adc_emu_probe(struct spi_device *spi)
 {
    struct iio_dev *indio_dev;
    struct iio_adc_emu_st *st;

    indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

    st = iio_priv(indio_dev);
    st->reg_select = 1;
    st->spi = spi;
    memset(st->chan_val, 0, sizeof(st->chan_val));
    indio_dev->name = "iio_adc_emu";
    indio_dev->info = &iio_adc_emu_info;
    indio_dev->channels = iio_adc_emu_channels;
    indio_dev->num_channels = ARRAY_SIZE(iio_adc_emu_channels);

    return devm_iio_device_register(&spi->dev, indio_dev);
 }

 static struct spi_driver iio_adc_emu_driver = {
    .driver = {
        .name = "iio_adc_emu"
    },
    .probe = iio_adc_emu_probe
 };
 module_spi_driver(iio_adc_emu_driver);


MODULE_AUTHOR("Cucu Maria-virginia <mariavirginia.cucu@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD5592");
MODULE_LICENSE("GPL v2");