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

void adesto_cs(void) { LPC_GPIO1->CLR = external_flash_pin; }

void adesto_ds(void) { LPC_GPIO1->SET = external_flash_pin; }

void initialize_accessible_pin(void) {
  LPC_IOCON->P0_6 &= ~0b111;
  LPC_GPIO0->DIR |= (1 << 6);
}

void CS_enable_accessible_pin(void) {
  initialize_accessible_pin();
  LPC_GPIO0->PIN &= ~(1 << 6);
}

void CS_disable_accessible_pin(void) { LPC_GPIO0->PIN |= (1 << 6); }

adesto_flash_id_s adesto_read_signature(void) {
  uint8_t dummy_data = 0xFF;
  adesto_flash_id_s data;

  external_flash_init();
  adesto_cs();
  CS_enable_accessible_pin();
  (void)spi2_exchange_byte(manufacturer_opcode);
  data.manufacturer_id = spi2_exchange_byte(dummy_data);
  data.device_id_1 = spi2_exchange_byte(dummy_data);
  data.device_id_2 = spi2_exchange_byte(dummy_data);
  data.extended_device_id = spi2_exchange_byte(dummy_data);
  adesto_ds();
  CS_disable_accessible_pin();

  return data;
}

void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 8;
  spi2_peripheral_init(spi_clock_mhz);

  // From the LPC schematics pdf, find the pin numbers connected to flash memory
  // Read table 84 from LPC User Manual and configure PIN functions for SPI2 pins
  // You can use gpio__construct_with_function() API from gpio.h
  //
  // Note: Configure only SCK2, MOSI2, MISO2.
  // CS will be a GPIO output pin(configure and setup direction)
  spi2_pin_init();

  while (1) {
    adesto_flash_id_s id = adesto_read_signature();
    // TODO: printf the members of the 'adesto_flash_id_s' struct
    printf("Manufaturer ID:0X%x \n", id.manufacturer_id);
    printf("Device 1 ID:0X%x \n", id.device_id_1);
    printf("Device 2 ID: 0X%x \n", id.device_id_2);
    printf("Extended device ID:0X%x \n", id.extended_device_id);
    vTaskDelay(500);
  }
}

void spi_id_verification_task(void *p) {
  while (1) {
    if (xSemaphoreTake(spi_bus_mutex, 1000)) {
      // fprintf(stderr, "Mq\n");
      const adesto_flash_id_s id = adesto_read_signature();
      printf("Manufaturer ID:0X%x \n", id.manufacturer_id);
      printf("Device 1 ID:0X%x \n", id.device_id_1);
      printf("Device 2 ID: 0X%x \n", id.device_id_2);
      printf("Extended device ID:0X%x \n", id.extended_device_id);
      xSemaphoreGive(spi_bus_mutex);

      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      }
    }

    // When we read a manufacturer ID we do not expect, we will kill this task

    // if (id.manufacturer_id != 0x1F) {
    //   fprintf(stderr, "Manufacturer ID read failure\n");
    //   vTaskSuspend(NULL); // Kill this task
    // }
  }
}

void main(void) {
  // Lab 5
  // xTaskCreate(spi_task, "spi", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  // Lab 5 Part 3
  // spi_bus_mutex = xSemaphoreCreateMutex();
  // spi2_peripheral_init(12);
  // spi2_pin_init;
  // xTaskCreate(spi_id_verification_task, "spi_test", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // xTaskCreate(spi_id_verification_task, "spi_test", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);



  puts("Starting RTOS..hello world..hey");


  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

}
