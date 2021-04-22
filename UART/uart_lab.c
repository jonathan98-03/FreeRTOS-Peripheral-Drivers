#include "uart_lab.h"
#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"

static const uint32_t DLAB = (1 << 7);
static const uint32_t PCUART2 = (1 << 24);
static const uint32_t PCUART3 = (1 << 25);

typedef LPC_UART_TypeDef lpc_uart;
static lpc_uart *uarts[] = {LPC_UART2, LPC_UART3};
static QueueHandle_t your_uart_rx_queue;

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  // fprintf(stderr, "lab init \n");
  power_on_UARTn(uart);
  enable_DLAB(uart);
  setup_DLL_DLM(uart, peripheral_clock, baud_rate);
  setup_FDR(uart);
  disable_DLAB(uart);

  // const uint8_t enable_fifo = (1 << 0); // Must be done!
  // const uint8_t eight_char_timeout = (2 << 6);
  // uarts[uart]->FCR = enable_fifo;
  // uarts[uart]->FCR = enable_fifo | eight_char_timeout;

  const uint32_t eight_bit_communication = 3;
  uarts[uart]->LCR = eight_bit_communication;
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  // disable_DLAB(uart);
  const uint32_t char_available_bit_mask = (1 << 0);
  while (!(uarts[uart]->LSR & char_available_bit_mask)) {
    ;
  }

  *input_byte = uarts[uart]->RBR;
  return true;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  // disable_DLAB(uart);

  const uint32_t trasmitter_empty = (1 << 5);
  while (!(uarts[uart]->LSR & trasmitter_empty)) {
    ;
  }
  uarts[uart]->THR = output_byte;
  while (!(uarts[uart]->LSR & trasmitter_empty)) {
    ;
  }
  return true;
}

static void your_receive_interrupt(void) {
  const uint32_t no_interrupt_pending = (1 << 0);
  const uint32_t RDA = (2 << 1);
  const uint32_t char_available_bit_mask = (1 << 0);

  while (LPC_UART3->IIR & no_interrupt_pending) {
    ;
  }

  if (LPC_UART3->IIR & RDA) {
    while (!(LPC_UART3->LSR & char_available_bit_mask)) {
      ;
    }

    const char byte = LPC_UART3->RBR;
    xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  } else {
    fprintf(stderr, "Not valid interrupt \n");
  }
}

static void your_receive_interrupt_2(void) {
  const uint32_t no_interrupt_pending = (1 << 0);
  const uint32_t RDA = (2 << 1);
  const uint32_t char_available_bit_mask = (1 << 0);

  while (LPC_UART2->IIR & no_interrupt_pending) {
    ;
  }

  if (LPC_UART2->IIR & RDA) {
    while (!(LPC_UART2->LSR & char_available_bit_mask)) {
      ;
    }

    const char byte = LPC_UART2->RBR;
    xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  } else {
    fprintf(stderr, "Not valid interrupt \n");
  }
}

void uart__enable_receive_interrupt(uart_number_e uart_number) {
  lpc_peripheral_e lpc_peripheral_type;

  if (uart_number) {
    lpc_peripheral_type = LPC_PERIPHERAL__UART3;
  } else {
    lpc_peripheral_type = LPC_PERIPHERAL__UART2;
  }

  lpc_peripheral__enable_interrupt(lpc_peripheral_type, your_receive_interrupt, "UART_3");
  lpc_peripheral__enable_interrupt(lpc_peripheral_type, your_receive_interrupt_2, "UART_2");

  if (uart_number) {
    NVIC_EnableIRQ(UART3_IRQn);
  } else {
    NVIC_EnableIRQ(UART2_IRQn);
  }

  const uint32_t RBRIE = (1 << 0);
  uarts[uart_number]->IER |= RBRIE;

  your_uart_rx_queue = xQueueCreate(16, sizeof(char));
}

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}

void power_on_UARTn(uart_number_e uart) {
  switch (uart) {
  case UART_2: {
    LPC_SC->PCONP |= PCUART2;
    break;
  }
  case UART_3: {
    LPC_SC->PCONP |= PCUART3;
    break;
  }
  default:
    break;
  }
}

void enable_DLAB(uart_number_e uart) { uarts[uart]->LCR = DLAB; }

void disable_DLAB(uart_number_e uart) { uarts[uart]->LCR &= ~DLAB; }

void setup_DLL_DLM(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {

  /* Baud rate equation from LPC user manual:
   * Baud = PCLK / (16 * (DLM*256 + DLL) * (1 + DIVADD/DIVMUL))
   *
   * What if we eliminate some unknowns to simplify?
   * Baud = PCLK / (16 * (DLM*256 + DLL) * (1 + 0/1))
   * Baud = PCLK / (16 * (DLM*256 + DLL)
   *
   * | DLM | DLL | is nothing but 16-bit number
   * DLM multiplied by 256 is just (DLM << 8)
   *
   * The equation is actually:
   * Baud = PCLK / 16 * (divider_16_bit)
   */
  const uint16_t divider_16_bit = peripheral_clock / (16 * baud_rate);

  uarts[uart]->DLM = (divider_16_bit >> 8) & 0xFF;
  uarts[uart]->DLL = (divider_16_bit >> 0) & 0xFF;
}

void setup_FDR(uart_number_e uart) {
  const uint32_t reset_mulval = (1 << 4); // Field must be greater or equal to 1 for UARTn to operate properly

  uarts[uart]->FDR = reset_mulval;
}

void uart2_pin_init(void) {
  LPC_IOCON->P2_8 &= ~0b111; // clear previously set bits if any
  LPC_IOCON->P2_9 &= ~0b111; // clear previously set bits if any

  LPC_IOCON->P2_8 |= 0b010;
  LPC_IOCON->P2_9 |= 0b010;
}

void uart3_pin_init(void) {
  LPC_IOCON->P4_28 &= ~0b111; // clear previously set bits if any
  LPC_IOCON->P4_29 &= ~0b111; // clear previously set bits if any

  LPC_IOCON->P4_28 |= 0b010;
  LPC_IOCON->P4_29 |= 0b010;
}
