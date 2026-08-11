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


#define AD5592R_S_MSB_MSK BIT(15)
#define AD5592R_S_ADDR_MSK GENMASK(14, 11)
#define AD5592R_S_DATA_MSK GENMASK(10, 0)

#define AD5592R_S_REG_RDB_ADDR 0x7
#define AD5592R_S_EN_READB BIT(6)
#define AD5592R_S_REG_SELECT_RDB GENMASK(5, 2)

#define AD5592R_S_REG_PD_ADDR 0xB
#define AD5592R_S_REG_EN_IREF BIT(9)

 struct ad5592r_s_st{
 	int reg_select;
	
	int chan_val[6];
	struct spi_device *spi;
 };

 

 static int ad5592r_s_spi_write(struct ad5592r_s_st *st, u8 addr, u16 data)
 {
	u16 tx = 0;
	u16 package = 0;

 	struct spi_transfer t = {
 		.tx_buf = &package,
 		.len = 2, //2 Bytes pentru ca avem 16 biti de trimis
 	};

	tx = FIELD_PREP(AD5592R_S_MSB_MSK, 0) | FIELD_PREP(AD5592R_S_ADDR_MSK, addr)
		 | FIELD_PREP(AD5592R_S_DATA_MSK, data); 

	put_unaligned_be16(tx, &package); //convertim datele in big endian pentru a fi trimise prin spi

	dev_info(&st->spi->dev, "tx we constructed: %x\n", tx);
	dev_info(&st->spi->dev, "package we constructed: %x\n", package);

 	return spi_sync_transfer(st->spi, &t, 1); //trimitem datele prin spi, 1 pentru ca avem 1 structura de tip transfer

 }

static int ad5592r_s_spi_read(struct ad5592r_s_st *st, u8 addr, u16 *data)
 {
	u16 rx = 0;
	u16 reg_rdb_data;
	u16 rcv_data = 0;
	int ret = 0;
 	struct spi_transfer t = {
			.tx_buf = NULL,
 			.rx_buf = &rx,
 			.len = 2,
 	};

	reg_rdb_data = FIELD_PREP(AD5592R_S_EN_READB, 1) | FIELD_PREP(AD5592R_S_REG_SELECT_RDB, addr);
	ret = ad5592r_s_spi_write(st, AD5592R_S_REG_RDB_ADDR, reg_rdb_data); //trimitem comanda de citire a registrului

	if(ret)
	{
		dev_info(&st->spi->dev, "Writing the readback register failed");
	}

	ret = spi_sync_transfer(st->spi, &t,1);

	if(ret)
	{
		dev_info(&st->spi->dev, "Failed receiving readback");
		return ret;
	}

	rcv_data = get_unaligned_be16(&rx);
	*data = FIELD_GET(AD5592R_S_DATA_MSK, rcv_data);

	return 0;
 }


 static int ad5592r_s_debugfs_reg_access(struct iio_dev *indio_dev, unsigned reg, 
											unsigned writeval, 
											unsigned *readval)
 {
	struct ad5592r_s_st *st = iio_priv(indio_dev);

	if (readval) {
		return ad5592r_s_spi_read(st, reg, (u16*) readval);
	}
	else {
		return ad5592r_s_spi_write(st, reg, writeval);
	}
	
 }

 static int ad5592r_s_read_raw(struct iio_dev *indio_dev, //folosim pentru a citi datele de la driver
 				struct iio_chan_spec const *chan,
 				int *val, //returnam valarea prin referinta pentru a putea fi modificata in functie de ce citim
				int *val2, //folosim pentru floating point
				long mask)
 {
	struct ad5592r_s_st *st = iio_priv(indio_dev); //folosim pentru a accesa structura noastra

 	switch (mask) {
 	case IIO_CHAN_INFO_RAW:
	if(!st->reg_select) {
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

  static int ad5592r_s_write_raw(struct iio_dev *indio_dev, //device(ar trebui sa fie canalul nostru), canalul, valoarea, valoarea2, masca
 				struct iio_chan_spec const *chan,
 				int val,
 				int val2,
 				long mask)
 {
	struct ad5592r_s_st *st = iio_priv(indio_dev); //folosim pentru a accesa structura noastra

		switch (mask) {
			case IIO_CHAN_INFO_RAW:
			if(!st->reg_select) {
				switch (chan->channel) {
				case 0:
					dev_info(&indio_dev->dev, "Trying to write to channel 0\n");
					st->chan_val[0] = val;
					break;
				case 1:
					dev_info(&indio_dev->dev, "Trying to write to channel 1\n");
					st->chan_val[1] = val;
					break;
				case 2:
					dev_info(&indio_dev->dev, "Trying to write to channel 2\n");
					st->chan_val[2] = val;
					break;
				case 3:
					dev_info(&indio_dev->dev, "Trying to write to channel 3\n");
					st->chan_val[3] = val;
					break;
				case 4:
					dev_info(&indio_dev->dev, "Trying to write to channel 4\n");
					st->chan_val[4] = val;
					break;
				case 5:
					dev_info(&indio_dev->dev, "Trying to write to channel 5\n");
					st->chan_val[5] = val;
					break;
				default:
					return -EINVAL;
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


  static const struct iio_chan_spec ad5592r_s_channels[] = { //confirurare canalelor, in cazul nostru 2 canale de tip tensiune
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
	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 2,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 3,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 4,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 	{
 		.type = IIO_VOLTAGE,
 		.indexed = 1,
 		.channel = 5,
 		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
 	},
 };


 static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
	.debugfs_reg_access = &ad5592r_s_debugfs_reg_access,
 };


 static int ad5592r_s_probe(struct spi_device *spi)  //primeste ca parametru struct pentru ca e driver de spi
 {
 	struct iio_dev *indio_dev;
	struct ad5592r_s_st *st;

 	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st)); //devm submodul linux, se ocupa de alocarea de memorie
	
	st = iio_priv(indio_dev); //returneaza pointerul catre structura noastra
	st->reg_select = 1;
	st->spi = spi;
	memset(st->chan_val, 0, sizeof(st->chan_val)); //initializam valorile canalelor cu 0
    indio_dev->name = "ad5592r_s";
    indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channels);

	ad5592r_s_spi_write(st, AD5592R_S_REG_PD_ADDR, FIELD_PREP(AD5592R_S_REG_EN_IREF, 1));
 	return devm_iio_device_register(&spi->dev, indio_dev);
 }

 struct spi_driver ad5592r_s_driver = {
 	.driver = {
 		.name = "ad5592r_s"
 	},
 	.probe = ad5592r_s_probe  //instantiaza driverul cu structura noastra
 };

 module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Pelin Mihai <mihai.pelin01@gmail.com>");
MODULE_DESCRIPTION("IIO-EMU SPI ADC driver");
MODULE_LICENSE("GPL v2");