// file gpio_lab.h
#pragma once

#include <stdbool.h>
#include <stdint.h>

// include this file at gpio_lab.c file
// #include "lpc40xx.h"

// NOTE: The IOCON is not part of this driver

/// Should alter the hardware registers to set the pin as input
void gpioN__set_as_input(uint8_t pin_num, uint8_t port_num);

/// Should alter the hardware registers to set the pin as output
void gpioN__set_as_output(uint8_t pin_num, uint8_t port_num);

/// Should alter the hardware registers to set the pin as high
void gpioN__set_high(uint8_t pin_num, uint8_t port_num);

/// Should alter the hardware registers to set the pin as low
void gpioN__set_low(uint8_t pin_num, uint8_t port_num);

// /**
//  * Should alter the hardware registers to set the pin as low
//  *
//  * @param {bool} high - true => set pin high, false => set pin low
//  */
void gpioN__set(uint8_t pin_num, uint8_t port_num, bool high);

// /**
//  * Should return the state of the pin (input or output, doesn't matter)
//  *
//  * @return {bool} level of pin high => true, low => false
//  */
bool gpioN__get_level(uint8_t pin_num, uint8_t port_num);
