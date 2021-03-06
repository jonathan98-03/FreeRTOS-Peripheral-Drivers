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

/**************************************************************************************************************************************
******************************************************************************************************************************************
************************************* LAB 2
*Code*******************************************************************************************
******************************************************************************************************************************************/

// Structure to make it easy to pass in port number and pin in task function parameters
typedef struct {
  /* First get gpioN driver to work only, and if you finish it
   * you can do the extra credit to also make it work for other Ports
   */
  uint8_t port;

  uint8_t pin;
} port_pin_s;

// PART 0: Basic structure to blink an LED
// void led_task_1(void *pvParameters) {
//   LPC_GPIO1->DIR |= pin26;

//   while (true) {
//     LPC_GPIO1->PIN |= pin26;
//     vTaskDelay(500);

//     puts("I am in");

//     LPC_GPIO1->PIN &= ~pin26;
//     vTaskDelay(500);
//   }
// }

// Part 1
// void led_task_2(void *task_parameter) {
//   // Type-cast the paramter that was passed from xTaskCreate()
//   const port_pin_s *led = (port_pin_s *)(task_parameter);
//   gpioN__set_as_output(led->pin, led->port);

//   while (true) {
//     gpioN__set_high(led->pin, led->port);
//     vTaskDelay(3000);

//     gpioN__set_low(led->pin, led->port);
//     vTaskDelay(3000);
//   }
// }

// Part 3
void led_task_3(void *task_parameter) {
  const port_pin_s *led = (port_pin_s *)(task_parameter);
  gpioN__set_as_output(led->pin, led->port);

  while (true) {
    // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore(signal)
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      // TODO: Blink the LED

      gpioN__set_high(led->pin, led->port);
      vTaskDelay(500);

      gpioN__set_low(led->pin, led->port);
      vTaskDelay(500);
    } else {
      puts("Timeout: No switch press indication for 1000ms");
    }
  }
}

void switch_task(void *task_parameter) {
  port_pin_s *switch1 = (port_pin_s *)task_parameter;
  gpioN__set_as_input(switch1->pin, switch1->port);

  while (true) {
    // TODO: If switch pressed, set the binary semaphore

    if (gpioN__get_level(switch1->pin, switch1->port)) {
      xSemaphoreGive(switch_press_indication);
    } else {
      puts("I am not in Semaphore");
    }

    // Task should always sleep otherwise they will use 100% CPU
    // This task sleep also helps avoid spurious semaphore give during switch debeounce
    vTaskDelay(100);
  }
}

void main(void) {
  // switch_press_indication = xSemaphoreCreateBinary();
  // static port_pin_s switch1 = {1, 15};
  // static port_pin_s led0 = {1, 24};
  // static port_pin_s led1 = {0, 0};

  // switch_pressed_signal = xSemaphoreCreateBinary();

  // while (1) {
  //   gpioN__set_as_output(1, 24);
  //   delay__ms(100);
  //   // TODO: Toggle an LED here
  //   gpioN__set_high(1, 24);
  //   vTaskDelay(500);

  //   gpioN__set_low(1, 24);
  //   vTaskDelay(500);
  // }

  puts("Starting RTOS..hello world..hey");

  // Run Part 3 code
  // xTaskCreate(switch_task, "switch", 2084 / sizeof(void *), &switch1, PRIORITY_LOW, NULL);
  // xTaskCreate(led_task_3, "led", 2084 / sizeof(void *), &led1, PRIORITY_LOW, NULL);

  // Run Part 1 code
  // xTaskCreate(led_task_2, "led", 2084 / sizeof(void *), &led0, PRIORITY_LOW, NULL);
  // xTaskCreate(led_task_2, "led", 2084 / sizeof(void *), &led1, PRIORITY_LOW, NULL);

  // xTaskCreate(task_one, "task_one", 1024, NULL, 1, NULL);
  // xTaskCreate(task_two, "task_two", 1024, NULL, 1, NULL);

  // Run Part 0 code
  // xTaskCreate(led_task_1, "led", 2084 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
}
