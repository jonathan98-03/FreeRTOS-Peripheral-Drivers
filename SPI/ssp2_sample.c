#include "ssp2_sample.h"
#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

static const external_flash_pin = (1 << 10);

void spi2_pin_init(void) {
  // Clear existing pin functions if any
  LPC_IOCON->P1_4 &= ~0b111;
  LPC_IOCON->P1_0 &= ~0b111;
  LPC_IOCON->P1_1 &= ~0b111;
  LPC_IOCON->P1_10 &= ~0b111;

  // Initialize pins to their appropriate functions
  LPC_IOCON->P1_4 |= 0b100; // SSP2_MISO
  LPC_IOCON->P1_0 |= 0b100; // SSP2_SCK
  LPC_IOCON->P1_1 |= 0b100; // SSP2_MOSI
}

void spi2_power_on(void) {
  const uint32_t spi2_peripheral = (1 << 20);
  LPC_SC->PCONP |= spi2_peripheral;
}

void spi2_CR_init(void) {
  uint32_t bits_8 = 0b111;
  const uint8_t SCR_SPI = (0b00 << 4);
  LPC_SSP2->CR0 |= bits_8 | SCR_SPI;

  const enable_spi = (0b1 << 1);
  LPC_SSP2->CR1 |= enable_spi;
}

void spi2_peripheral_init(uint32_t spi_clock_mhz) {
  spi2_power_on;
  spi2_CR_init;

  // LPC_SSP2->CPSR = 4; // 96/4 = 24Mhz
  uint8_t divider = 2;
  const uint32_t cpu_clock_mhz = clock__get_core_clock_hz() / 1000000UL;

  while (spi_clock_mhz < (cpu_clock_mhz / divider) && divider <= 254) {
    divider += 2;
  }

  LPC_SSP2->CPSR = divider;
}

uint8_t spi2_exchange_byte(uint8_t data_out) {
  const uint32_t busy = (1 << 4);
  LPC_SSP2->DR = data_out;

  while (LPC_SSP2->SR & busy) {
    ;
  }

  return LPC_SSP2->DR;
}

void external_flash_init(void) {
  LPC_GPIO1->DIR |= external_flash_pin;
  LPC_GPIO1->SET = external_flash_pin;
}
