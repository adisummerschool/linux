// SPDX-License-Identifier: GPL-2.0-only
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2025 Analog Devices Inc.
 */

 #include <linux/unaligned.h>
 #include <linux/bitfield.h>
 #include <linux/module.h>

 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>
 #include <linux/iio/triggered_buffer.h>
 #include <linux/iio/trigger_consumer.h>


 #define EMU_RDWR_MASK 			BIT(7)
 #define EMU_ADDR_MASK 			GENMASK(14, 8)
 #define EMU_DATA_MASK			GENMASK(7, 0)

 #define EMU_POWER_DISABLE 		0x20
 #define EMU_POWER_ENABLE 		0x0
 #define EMU_POWER_REG 			0x02

#define EMU_REG_CNVST 			0x03
#define EMU_CNVST_START			BIT(0)

#define EMU_REG_CHAN_HIGH(x) 	(0x04 + (2 * (x)) )
#define EMU_REG_CHAN_LOW(x)  	(0x05 + (2 * (x)) )

#define EMU_HIGH_DATA_MSK		GENMASK(11,8)

 struct iio_adc_emu_st {
	int reg_select;
	int chan_val[2];
	struct spi_device *spi;
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
		dev_info(&st->spi->dev, "SPI READ transfer failed");
		return ret;
	}

	*data = rx; //extragem datele din rx
	return 0; 
 }

 static int iio_adc_emu_spi_write(struct iio_adc_emu_st *st, u8 addr, u8 data)
 {
	u16 tx = 0;
	u16 package = 0;

 	struct spi_transfer t = {
 		.tx_buf = &package,
 		.len = 2, //pentru ca vrem sa trimitem adresa si date
 	};

	tx = FIELD_PREP(EMU_RDWR_MASK, 0) | FIELD_PREP(EMU_ADDR_MASK, addr)
		 | FIELD_PREP(EMU_DATA_MASK, data); 

	put_unaligned_be16(tx, &package); //convertim datele in big endian pentru a fi trimise prin spi

	//dev_info(&st->spi->dev, "tx we constructed: %x\n", tx);
	//dev_info(&st->spi->dev, "package we constructed: %x\n", package);

 	return spi_sync_transfer(st->spi, &t, 1); //trimitem datele prin spi, 1 pentru ca avem 1 structura de tip transfer

 }

 static int iio_adc_emu_read_chan(struct iio_adc_emu_st *st, int channel, u16 *readval)
 {
	u8 high, low; //ce citim in functie de ce canal avem
	u16  data = 0;
	int ret;

	ret = iio_adc_emu_spi_write(st, EMU_REG_CNVST, FIELD_PREP(EMU_CNVST_START, 1));
	if(ret){
		dev_err(&st->spi->dev, "Writing conversion reg failed %d\n", ret);
		return ret;
	}

	ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_HIGH(channel), &high);
	if(ret){
		dev_err(&st->spi->dev, "Reading high reg failed %d\n", ret);
		return ret;
	}

	ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_LOW(channel), &low);
	if(ret){
		dev_err(&st->spi->dev, "Reading low reg failed %d\n", ret);
		return ret;
	}

	data = FIELD_PREP(EMU_HIGH_DATA_MSK, high) | low;
	*readval = data;
	return 0;

 }

 static int iio_adc_emu_debugfs_reg_access(struct iio_dev *indio_dev, unsigned reg, 
											unsigned writeval, 
											unsigned *readval)
 {
	struct iio_adc_emu_st *st = iio_priv(indio_dev);

	if (readval) {
		return iio_adc_emu_spi_read(st, reg, (u8 *) readval);
	}
	else {
		return iio_adc_emu_spi_write(st, reg, writeval);
	}
	
 }



 static int iio_adc_emu_read_raw(struct iio_dev *indio_dev, //folosim pentru a citi datele de la driver
 				struct iio_chan_spec const *chan,
 				int *val, //returnam valarea prin referinta pentru a putea fi modificata in functie de ce citim
				int *val2, //folosim pentru floating point
				long mask)
 {
	struct iio_adc_emu_st *st = iio_priv(indio_dev); //returneaza pointer la structura privata a device-ului
	int ret;

 	switch (mask) {
		case IIO_CHAN_INFO_RAW:
			if(!st->reg_select){

					ret = iio_adc_emu_read_chan(st, chan->channel, (u16 *) val);
					if(ret){
						dev_err(&st->spi->dev, "Reading from channels failed");
						return ret;
					}
				return IIO_VAL_INT;	
			}
			else
				return -EINVAL;

		case IIO_CHAN_INFO_ENABLE:
				*val= st->reg_select;
				return IIO_VAL_INT;
		default:
			return -EINVAL;
 	}
 }



 static int iio_adc_emu_write_raw(struct iio_dev *indio_dev, //device(ar trebui sa fie canalul nostru), canalul, valoarea, valoarea2, masca
 				struct iio_chan_spec const *chan,
 				int val,
 				int val2,
 				long mask)
 {
	struct iio_adc_emu_st *st = iio_priv(indio_dev); //returneaza pointer la structura privata a device-ului
		switch (mask) {
			case IIO_CHAN_INFO_RAW:
				if(!st->reg_select){
					if(chan->channel)
					{
						dev_info(&indio_dev->dev, "Trying to write to channel 1\n");
						//st->chan_val[1] = val;
					}
					else
					{
						dev_info(&indio_dev->dev, "Trying to write to channel 0\n");
						//st->chan_val[0] = val;
					}
					return 0;
				}
				else
					return -EINVAL;
			case IIO_CHAN_INFO_ENABLE:
					if(val){
						st->reg_select = 1;
						iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_DISABLE); //disable power register
					}
					else{
						st->reg_select = 0;
						iio_adc_emu_spi_write(st, EMU_POWER_REG, EMU_POWER_ENABLE); //enable power register
					}
					return 0;
			default:
				return -EINVAL;
			}
 }

 static irqreturn_t iio_adc_emu_trigger_handler(int irq, void *p){

	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct iio_adc_emu_st *st = iio_priv(indio_dev);
	int bit = 0;
	int ret;
	u8 high, low;
	u16 buf[2];
	int i =0;

	ret =iio_adc_emu_spi_write(st, EMU_REG_CNVST, FIELD_PREP(EMU_CNVST_START, 1));

	if(ret){
		dev_err(&st->spi->dev, "Reading from channels failed");
		iio_trigger_notify_done(indio_dev->trig);
		return IRQ_HANDLED;
	}
	for_each_set_bit(bit, indio_dev->active_scan_mask, indio_dev->num_channels)
	{
		ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_HIGH(bit), &high);
		if(ret){
			dev_err(&st->spi->dev, "Reading high register failed in trigger: %d\n", ret);
			iio_trigger_notify_done(indio_dev->trig);
			return IRQ_HANDLED;
		}

		ret = iio_adc_emu_spi_read(st, EMU_REG_CHAN_LOW(bit), &low);
		if(ret){
			dev_err(&st->spi->dev, "Reading low register failed in trigger: %d\n", ret);
			iio_trigger_notify_done(indio_dev->trig);
			return IRQ_HANDLED;
		}
		buf[i++]= FIELD_PREP(EMU_HIGH_DATA_MSK, high) | low;
	}
	iio_push_to_buffers(indio_dev, buf);
	iio_trigger_notify_done(indio_dev->trig);
	return IRQ_HANDLED;
 }

 static const struct iio_chan_spec iio_adc_emu_channels[] = { //confirurare canalelor, in cazul nostru 2 canale de tip tensiune
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 0,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 0,
		.scan_type = {
			.sign ='u',
			.realbits = 12,
			.storagebits = 16
		}
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 1,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
		.scan_index = 1,
		.scan_type = {
			.sign ='u',
			.realbits = 12,
			.storagebits = 16
		}
 	},
 };

 static const struct iio_info iio_adc_emu_info = {
	.read_raw = &iio_adc_emu_read_raw,
	.write_raw = &iio_adc_emu_write_raw,
	.debugfs_reg_access = &iio_adc_emu_debugfs_reg_access,
 };


 static int iio_adc_emu_probe(struct spi_device *spi)  //primeste ca parametru struct pentru ca e driver de spi
 {
 	struct iio_dev *indio_dev;
	struct iio_adc_emu_st *st;
	int ret;

 	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st)); //devm submodul linux, se ocupa de alocarea de memorie

	st = iio_priv(indio_dev); //returneaza pointer la structura privata a device-ului
	st->reg_select = 1;  //enable cu valoarea 1 (oprit)
	st->spi = spi; //initializam structura cu device-ul spi
	memset(st->chan_val, 0, sizeof(st->chan_val)); //initializam valorile canalelor cu 0
    indio_dev->name = "iio_adc_emu";
    indio_dev->info = &iio_adc_emu_info;
	indio_dev->channels = iio_adc_emu_channels;
	indio_dev->num_channels = ARRAY_SIZE(iio_adc_emu_channels);

	ret = devm_iio_triggered_buffer_setup(&spi->dev, indio_dev, NULL, &iio_adc_emu_trigger_handler, NULL);
	if(ret){
		dev_err(&spi->dev, "failed to create buffer");
		return ret;
	}
 	return devm_iio_device_register(&spi->dev, indio_dev);
 }

 struct spi_driver iio_adc_emu_driver = {
 	.driver = {
 		.name = "iio_adc_emu"
 	},
 	.probe = iio_adc_emu_probe  //instantiaza driverul cu structura noastra
 };

 module_spi_driver(iio_adc_emu_driver);

MODULE_AUTHOR("Pelin Mihai <mihai.pelin01@gmail.com>");
MODULE_DESCRIPTION("IIO-EMU SPI ADC driver");
MODULE_LICENSE("GPL v2");