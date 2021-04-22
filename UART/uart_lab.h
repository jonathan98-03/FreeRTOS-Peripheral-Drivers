#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  UART_2,
  UART_3,
} uart_number_e;

// Refer to LPC User manual and setup the register bits correctly
// The first page of the UART chapter has good instructions
// a) Power on Peripheral
// b) Setup DLL, DLM, FDR, LCR registers
void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate);

// Read the byte from RBR and actually save it to the pointer
// a) Check LSR for Receive Data Ready
// b) Copy data from RBR register to input_byte
bool uart_lab__polled_get(uart_number_e uart, char *input_byte);

// a) Check LSR for Transmit Hold Register Empty
// b) Copy output_byte to THR register
bool uart_lab__polled_put(uart_number_e uart, char output_byte);

static void your_receive_interrupt(void);
static void your_receive_interrupt_2(void);

void uart__enable_receive_interrupt(uart_number_e uart_number);

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout);

void power_on_UARTn(uart_number_e uart);

void enable_DLAB(uart_number_e uart);

void disable_DLAB(uart_number_e uart);

void setup_DLL_DLM(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate);

void setup_FDR(uart_number_e uart);

void uart2_pin_init(void);

void uart3_pin_init(void);
