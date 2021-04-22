#include "acceleration.h"
#include "adc.h"
#include "cli_handlers.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "i2c.h"
#include "i2c_slave_functions.h"
#include "i2c_slave_init.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "pwm1.h"
#include "sensors.h"
#include "ssp2_sample.h"
#include "uart_lab.h"
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);

static void task_one(void *task_parameter);
static void task_two(void *task_parameter);

static void led_task(void *pvParameters);

static const uint32_t pin26 = (1 << 26);
static const uint32_t pin30 = (1 << 30);

static const uint32_t external_flash_pin = (1 << 10);
static const uint8_t manufacturer_opcode = 0x9F;

static void gpio_interrupt(void);

static SemaphoreHandle_t switch_press_indication;
static SemaphoreHandle_t switch_pressed_signal;
static QueueHandle_t adc_to_pwm_task_queue;
static SemaphoreHandle_t spi_bus_mutex;

typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

void uart_read_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_get() function and printf the received value
    char input_byte;
    // uart_lab__polled_get(UART_3, &input_byte);
    // printf("Char received: %c \n", input_byte);
    if (uart_lab__get_char_from_queue(&input_byte, portMAX_DELAY)) {
      fprintf(stderr, "Input: %c \n", input_byte);
    }
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_put() function and send a value
    fprintf(stderr, "I am in write task \n");
    uart_lab__polled_put(UART_3, 'a');
    vTaskDelay(500);
  }
}

void board_1_sender_task(void *p) {
  char number_as_string[16] = {0};

  while (true) {
    const int number = rand();
    sprintf(number_as_string, "%i", number);

    // Send one char at a time to the other board including terminating NULL char
    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_3, number_as_string[i]);
      printf("Sent: %c\n", number_as_string[i]);
    }

    printf("Sent: %i over UART to the other board\n", number);
    vTaskDelay(3000);
  }
}

void board_2_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;

  // vTaskDelay(500);
  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    vTaskDelay(50);
    printf("Received: %c\n", byte);

    // This is the last char, so print the number
    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      vTaskDelay(50);
      printf("Received this number from the other board: %s\n", number_as_string);
    }
    // We have not yet received the NULL '\0' char, so buffer the data
    else {
      // TODO: Store data to number_as_string[] array one char at a time
      // Hint: Use counter as an index, and increment it as long as we do not reach max value of 16
      if (counter < 16) {
        number_as_string[counter] = byte;
        counter++;
      } else {
        fprintf(stderr, "Over array limit \n");
      }
    }
  }
}

void main(void) {



  // Lab 6
  // puts("hello \n");
  // uart_lab__init(UART_2, 96 * 1000 * 1000, 38400);
  // uart_lab__init(UART_3, 96 * 1000 * 1000, 38400);
  // uart3_pin_init();
  // xTaskCreate(uart_read_task, "read_task", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // xTaskCreate(uart_write_task, "write_task", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  // Lab 6 part 2
  // uart_lab__init(UART_3, 96 * 1000 * 1000, 38400);
  // uart3_pin_init();
  // xTaskCreate(uart_write_task, "write_task", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // xTaskCreate(uart_read_task, "read_task", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // fprintf(stderr, "Enable receive interrupt \n");
  // uart__enable_receive_interrupt(UART_3);

  // Lab 6 Part 3
  // uart_lab__init(UART_3, 96 * 1000 * 1000, 38400);
  // uart_lab__init(UART_2, 96 * 1000 * 1000, 38400);
  // uart3_pin_init();
  // uart2_pin_init();
  // uart__enable_receive_interrupt(UART_2);
  // xTaskCreate(board_1_sender_task, "write_task", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // xTaskCreate(board_2_receiver_task, "read_task", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);





  puts("Starting RTOS..hello world..hey");


  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
}
