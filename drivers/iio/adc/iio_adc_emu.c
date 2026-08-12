// SPDX-License Identifier: GPL-2.0
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2026 Analog Devices Inc.
 */

#include <linux/unaligned.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/bitfield.h>
#include<linux/iio/triggered_buffer.h>
#include<linux/iio/trigger_consumer.h>

#define EMU_RDWR_MASK BIT(7)
#define EMU_ADDR_MSK GENMASK(14, 8)
#define EMU_DATA_MSK GENMASK(7, 0)

#define EMU_POWER_REG 0x02
#define EMU_POWER_ENABLE 0x0
#define EMU_POWER_DISABLE 0x20

#define EMU_REG_CNVST 0x03 //registrul de conversie
#define EMU_CNVST_START BIT(0) //bit ul de start conversie

#define EMU_REG_CHAN_HIGH(x) 	(0x04 + (2 * (x)) )
#define EMU_REG_CHAN_LOW(x)  	(0x05 + (2 * (x)) )	

#define EMU_HIGH_DATA_MSK		GENMASK(11,8)

struct iio_adc_emu_st {
	struct spi_device *spi;
	bool reg_select;
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
 			.len = 1, //pentru ca vrem sa trimitem doar adresa
 		},
 		{
 			.rx_buf = &rx,
 			.len = 1,
 		},
 	};

	tx = FIELD_PREP(EMU_RDWR_MASK, 1) | addr; //setam bitul de read/write pe 1 pentru a citi si adresa pe care vrem sa o citim
	
 	ret = spi_sync_transfer(st->spi, t, 2); //trimitem datele prin spi, 2 pentru ca avem 2 structuri de tip transfer

	if(ret)
	{
		// dev_info(&st->spi->dev, "SPI READ transfer failed");
		return ret;
	}

	*data = rx; //extragem datele din rx
	return 0; 
 }

static int iio_adc_emu_spi_write(struct iio_adc_emu_st *st, u8 addr, u8 data)
{
	u16 tx = 0;
	u16 package = 0;
	struct spi_transfer t = { .tx_buf = &package, .len = 2 };

	tx = FIELD_PREP(EMU_RDWR_MASK, 0) | FIELD_PREP(EMU_ADDR_MSK, addr) |
	     FIELD_PREP(EMU_DATA_MSK, data);
	put_unaligned_be16(tx, &package);

	// dev_info(&st->spi->dev,
	// 	 "Constructed TX:0x%02x \nConstructed PACKAGE:0x%02x", tx,
	// 	 package);

	return spi_sync_transfer(st->spi, &t, 1);
}

static int iio_adc_emu_read_chan(struct iio_adc_emu_st *st, int channel, u16 *readval)
 {
	u8 high, low; //ce citim in functie de ce canal avem
	u16  data = 0;
	int ret;

	ret = iio_adc_emu_spi_write(st, EMU_REG_CNVST, FIELD_PREP(EMU_CNVST_START, 1)); //incepe conversia 
	if(ret){ 
		dev_err(&st->spi->dev, "Writing conversion reg failed %d\n", ret);
		return ret;
	}

	ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_HIGH(channel), &high);// citeste 4 biti pt HIGH 
	if(ret){
		dev_err(&st->spi->dev, "Reading high reg failed %d\n", ret);
		return ret;
	}

	ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_LOW(channel), &low); //citeste byte ul LOW
	if(ret){
		dev_err(&st->spi->dev, "Reading low reg failed %d\n", ret);
		return ret;
	}

	data = FIELD_PREP(EMU_HIGH_DATA_MSK, high) | low; //concatenarea celor 2 bytes
	*readval = data;
	return 0;
	

 }

static int iio_adc_emu_debugfs_reg_access(struct iio_dev *indio_dev,
					  unsigned reg, unsigned writeval,
					  unsigned *readval)
{
	struct iio_adc_emu_st *st = iio_priv(indio_dev);

	if (readval) {
		return iio_adc_emu_spi_read(st, reg, (u8 *)readval);
	}
	return iio_adc_emu_spi_write(st, reg, writeval);
}

static irqreturn_t iio_adc_emu_trigger_handler(int irq, void *p){
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct iio_adc_emu_st *st = iio_priv(indio_dev);
	// indio_dev -> active_scan_mask daca bitu 0 e activ canalu 0 e activ
	int bit;
	int ret;
	u8 high,low;
	u16 buf[2];
	int i = 0;
		ret = iio_adc_emu_spi_write(st, EMU_REG_CNVST, FIELD_PREP(EMU_CNVST_START, 1)); //incepe conversia 
	if(ret){ 
		dev_err(&st->spi->dev, "Writing conversion reg failed %d\n", ret);
		iio_trigger_notify_done(indio_dev->trig); 
		return IRQ_HANDLED;
	}
	for_each_set_bit(bit,indio_dev->active_scan_mask,indio_dev->num_channels){

			ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_HIGH(bit), &high);// citeste 4 biti pt HIGH 
	if(ret){
		dev_err(&st->spi->dev, "Reading high reg failed in trigger%d\n", ret);
		iio_trigger_notify_done(indio_dev->trig); 
		return IRQ_HANDLED;
	}

	ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_LOW(bit), &low); //citeste byte ul LOW
	if(ret){
		dev_err(&st->spi->dev, "Reading low reg failed in trigger %d\n", ret);
		iio_trigger_notify_done(indio_dev->trig); 
		return IRQ_HANDLED;
	}
		buf[i++] = FIELD_PREP(EMU_HIGH_DATA_MSK, high) | low;
	}
	iio_push_to_buffers(indio_dev,buf);
	iio_trigger_notify_done(indio_dev->trig); 
	return IRQ_HANDLED;
}

// structura pentru canalele ADC
static const struct iio_chan_spec iio_adc_emu_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
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
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.scan_index = 1,
		.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 16
		}
	}
};

// citim datele de la ADC
static int iio_adc_emu_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan, int *val,
				int *val2, long mask)
{
	// citim datele
	struct iio_adc_emu_st *st = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		// citim datele de la ADC
		if (!st->reg_select) {
				//channel 1
				ret = iio_adc_emu_read_chan(st, chan->channel,(u16 *)val); //hardcoded value for channel 1
				//channel 0
				if(ret){
					dev_err(&st->spi->dev, "Reading from channels failed");
					return ret;
				}
			return IIO_VAL_INT;
		} else {
			return -EINVAL;
		}
	case IIO_CHAN_INFO_ENABLE:
		*val = st->reg_select ? 1 : 0;
		return IIO_VAL_INT;
	default:
		//returnam datele citite
		return -EINVAL;
	}
	return 0;
}

static int iio_adc_emu_write_raw(struct iio_dev *indio_dev,
				 struct iio_chan_spec const *chan, int val,
				 int val2, long mask)
{
	// scriem datele
	struct iio_adc_emu_st *st = iio_priv(indio_dev);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (!st->reg_select) {
			if (chan->channel) {
				// st->chan_val[1] = val;
				// dev_info(&indio_dev->dev,
				// 	 "Trying to write to channel 1");
			} else {
				// st->chan_val[0] = val;
				// dev_info(&indio_dev->dev,
				// 	 "Trying to write to channel 0");
			}
			return 0;
		} else {
			return -EINVAL;
		}
	case IIO_CHAN_INFO_ENABLE:
		if(val){
		st->reg_select = 1;
		iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_DISABLE);
		}else{
		st->reg_select = 0;
		iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_ENABLE);
		}

		return 0;
	default:
		return -EINVAL;
	}
	return 0;
	//returnam datele scrise
}
static const struct iio_info iio_adc_emu_info = {
	.read_raw = &iio_adc_emu_read_raw,
	.write_raw = &iio_adc_emu_write_raw,
	.debugfs_reg_access = iio_adc_emu_debugfs_reg_access
};
// initializam driverul
static int iio_adc_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct iio_adc_emu_st *st;
	int ret;
	//pentru alocarea memoriei
	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	st = iio_priv(indio_dev);
	st->reg_select = true;
	st->spi = spi;
	memset(st->chan_val, 0, sizeof(st->chan_val));
	//popularea structurii
	indio_dev->name = "iio_adc_emu";
	indio_dev->info = &iio_adc_emu_info;
	indio_dev->channels = iio_adc_emu_channels;
	indio_dev->num_channels = 2;

	ret = devm_iio_triggered_buffer_setup(&spi->dev,indio_dev,NULL, &iio_adc_emu_trigger_handler,NULL);
	if(ret){
		dev_err(&spi->dev, "Filed to create buffer");
		return ret;
	}

	return devm_iio_device_register(&spi->dev, indio_dev);
}

// definim driverul SPI
static struct spi_driver iio_adc_emu_driver = {
	.driver = { .name = "iio_adc_emu" },
	.probe = iio_adc_emu_probe
};
module_spi_driver(iio_adc_emu_driver);

MODULE_AUTHOR("Tomescu Lucas <lucastomescu@gmail.com>");
MODULE_DESCRIPTION("Analog Devices IIO ADC EMU Summer School");
MODULE_LICENSE("GPL v2");
