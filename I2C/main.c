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

void main(void) {


  // printf("hello \n");
  // Lab 9
   i2c0__slave_init(0x86);
   sj2_cli__init();








  puts("Starting RTOS..hello world..hey");



}
