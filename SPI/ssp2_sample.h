
#pragma once

#include <stdint.h>
#include <stdio.h>

void spi2_pin_init(void);

void spi2_power_on(void);

void spi2_CR_init(void);

void spi2_peripheral_init(uint32_t spi_clock_mhz);

void external_flash_init(void);

uint8_t spi2_exchange_byte(uint8_t data_out);
