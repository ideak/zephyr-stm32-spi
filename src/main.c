/*
 * SPDX-License-Identifier: SPDX-License-Identifier: Apache-2.0
 * Author: Imre Deak <imre.deak@gmail.com>
 *
 * This simple app demonstrates how to use Zephyr for SPI transfers.
 *
 * It uses the SPI1 bus which on the STM32F411RE has the following pinout
 * configuration (by default configured that way by the Zephyr core):
 *
 *   SPI1_SCK:  PA5	(alternative pin: PB3)
 *   SPI1_MISO: PA6	(alternative pin: PB4)
 *   SPI1_MOSI: PA7	(alternative pin: PB5)
 *   SPI1_NSS:  PA4	(alternative pin: PA15)
 *
 * See
 *
 *	https://www.st.com/resource/en/datasheet/stm32f411ce.pdf
 *
 * Chapter 4 "Pinouts and pin description" for more details on configuring the
 * pins for alternate functions and
 *
 *	https://os.mbed.com/platforms/ST-Nucleo-F411RE
 *
 * "Morpho headers" for locating the pins on the board.
 *
 * The SPI bus will be used in master (non-TI aka Motorola) mode, see the
 * STM32F411xC/E reference manual (RM0383) at
 *
 *	https://www.st.com/resource/en/reference_manual/dm00119316.pdf
 *
 * Chapter 20.3.3 "Configuring the SPI in master mode" and
 *         20.3.5 "Data transmission and reception procedures"/
 *		   "Start sequence in master mode"
 * for the corresponding low-level programming (done by Zephyr internally).
 *
 * For simplicity the NSS line (chip select) will be operated in
 * "hardware mode" see the reference manual Chapter 20.3.1 in general and
 * within that specifically the "Slave select (NSS) pin management" part. In
 * practice this means that the HW itself will assert the NSS line whenever it
 * transmits data and de-asserts it at the end of the transfer.
 *
 * For the register flags related to the above master mode and NSS setup see
 * the reference manual
 * Chapter 20.5.1 "SPI control register 1 (SPI_CR1)" SSM/SSI/MSTR and
 * Chapter 20.5.2 "SPI control register 2 (SPI_CR2)" SSOE flags.
 *
 * The NSS line needs to be pulled up but Zephyr doesn't configure an internal
 * pull-up for the corresponding PA4 pin by default. Since this app is geared
 * for an easy setup-and-measure with a scope scenario it will configure an
 * internal pull-up on the pin during init in stm32_spi_setup_nss_pin().
 *
 * If your circuit has an external pull-up resistor then you don't need (or
 * even should not) configure the internal one (achieved by not calling
 * stm32_spi_setup_nss_pin()).
 *
 * As an alternative to the above NSS "hardware mode" it's also possible to
 * use any available pin in GPIO mode to provide for the SPI chip select
 * functionality. In this case struct spi_config::cs needs to be initialized
 * appropriately in main(), which will make Zephyr configure the SPI module in
 * the NSS "software mode". See the reference manual for details for the
 * corresponding low-level programming. This app itself would then need to
 * assert/de-assert the given GPIO around each SPI transfer.
 *
 * Probably for a real use case you want to use the NSS "software mode"
 * because that's the only way you can:
 * - control the timing of the chip select signal
 * - configure the CS signal as positive-asserted (rare case)
 * - use more than one CS signal to control multiple slaves
 * The "hardware mode" uses some kind of a fixed timing, supports only the
 * standard negative-asserted CS logic and a single CS signal.
 */
#include <string.h>
#include <spi.h>

#include <pinmux/stm32/pinmux_stm32.h>

static int stm32_spi_send(struct device *spi,
			  const struct spi_config *spi_cfg,
			  const uint8_t *data, size_t len)
{
	const struct spi_buf_set tx = {
		.buffers = &(const struct spi_buf){
			.buf = (uint8_t *)data,
			.len = len,
		},
		.count = 1,
	};

	return spi_write(spi, spi_cfg, &tx);
}

static int stm32_spi_send_str(struct device *spi,
			      const struct spi_config *spi_cfg,
			      const unsigned char *str)
{
	return stm32_spi_send(spi, spi_cfg, str, strlen(str));
}

static void stm32_spi_setup_nss_pin(void)
{
	static const struct pin_config pin_config = {
		.pin_num = STM32_PIN_PA4,
		.mode = STM32_PINMUX_ALT_FUNC_5 | STM32_PUSHPULL_PULLUP,
	};

	stm32_setup_pins(&pin_config, 1);
}

void main(void)
{
	struct spi_config spi_cfg = {};
	struct device *spi;
	int err;

	printk("Starting stm32-spi on %s\n", CONFIG_BOARD);

	stm32_spi_setup_nss_pin();

	spi = device_get_binding("SPI_1");
	if (!spi) {
		printk("Could not find SPI driver\n");
		return;
	}

	spi_cfg.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
			    SPI_OP_MODE_MASTER;

	spi_cfg.frequency = 1000000U;

	for (;;) {
		err = stm32_spi_send_str(spi, &spi_cfg, "HELLO!");
		if (err)
			break;
		k_sleep(1);
	}

	if (err)
		printk("SPI send error %d\n", err);
}
