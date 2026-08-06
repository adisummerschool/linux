// SPDX-License-Identifier: GPL-2.0-only
/*
 * IIO-EMU SPI ADC driver
 *
 * Copyright 2025 Analog Devices Inc.
 */

 #include <linux/module.h>
 #include <linux/spi/spi.h>
 #include <linux/iio/iio.h>

 struct iio_adc_emu_st {
	int reg_select;
	int chan_val[2];
 }; 


 static int iio_adc_emu_read_raw(struct iio_dev *indio_dev, //folosim pentru a citi datele de la driver
 				struct iio_chan_spec const *chan,
 				int *val, //returnam valarea prin referinta pentru a putea fi modificata in functie de ce citim
				int *val2, //folosim pentru floating point
				long mask)
 {
	struct iio_adc_emu_st *st = iio_priv(indio_dev); //returneaza pointer la structura privata a device-ului
 	switch (mask) {
		case IIO_CHAN_INFO_RAW:
			if(!st->reg_select){
				if(chan->channel)
					*val = st->chan_val[1];
				else 
					*val = st->chan_val[0];

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
						st->chan_val[1] = val;
					}
					else
					{
						dev_info(&indio_dev->dev, "Trying to write to channel 0\n");
						st->chan_val[0] = val;
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

 static const struct iio_chan_spec iio_adc_emu_channels[] = { //confirurare canalelor, in cazul nostru 2 canale de tip tensiune
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 0,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 1,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 };

 static const struct iio_info iio_adc_emu_info = {
	.read_raw = &iio_adc_emu_read_raw,
	.write_raw = &iio_adc_emu_write_raw,
 };


 static int iio_adc_emu_probe(struct spi_device *spi)  //primeste ca parametru struct pentru ca e driver de spi
 {
 	struct iio_dev *indio_dev;
	struct iio_adc_emu_st *st;

 	int ret;

 	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st)); //devm submodul linux, se ocupa de alocarea de memorie

	st = iio_priv(indio_dev); //returneaza pointer la structura privata a device-ului
	st->reg_select = 1;  //enable cu valoarea 1 (oprit)
	memset(st->chan_val, 0, sizeof(st->chan_val)); //initializam valorile canalelor cu 0
    indio_dev->name = "iio_adc_emu";
    indio_dev->info = &iio_adc_emu_info;
	indio_dev->channels = iio_adc_emu_channels;
	indio_dev->num_channels = ARRAY_SIZE(iio_adc_emu_channels);


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