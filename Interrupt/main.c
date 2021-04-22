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

//[lab 3]--
// void gpio_interrupt(void) {
//   LPC_GPIOINT->IO0IntClr = pin30;
//   fprintf(stderr, "I am in the interrupt");
// }

void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  LPC_GPIOINT->IO0IntClr = pin30;
}

void sleep_on_sem_task(void *p) {
  while (1) {
    // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
    if (xSemaphoreTake(switch_pressed_signal, portMAX_DELAY)) {

      // Set Port 1 pin 24 as output
      LPC_GPIO1->DIR |= (1 << 24);

      // Light up LED
      LPC_GPIO1->PIN |= (1 << 24);
      vTaskDelay(500);
      LPC_GPIO1->PIN &= ~(1 << 24);
      vTaskDelay(500);
    }
  }
}

void pin30_isr(void) {
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  // LPC_GPIOINT->IO0IntClr = (1 << 30);
  fprintf(stderr, "I am in pin30_isr \n");
}

void pin29_isr(void) {
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  // LPC_GPIOINT->IO0IntClr = (1 << 29);
  fprintf(stderr, "I am in pin29_isr \n");
}

void led_task(void *p) {
  while (1) {
    // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
    if (xSemaphoreTake(switch_pressed_signal, portMAX_DELAY)) {

      // Set Port 1 pin 24 as output
      LPC_GPIO1->DIR |= (1 << 24);

      // Light up LED
      LPC_GPIO1->PIN |= (1 << 24);
      vTaskDelay(500);
      LPC_GPIO1->PIN &= ~(1 << 24);
      vTaskDelay(500);
    }
  }
}

void main(void) {
  // switch_press_indication = xSemaphoreCreateBinary();
  // static port_pin_s switch1 = {1, 15};
  // static port_pin_s led0 = {1, 24};
  // static port_pin_s led1 = {0, 0};

  // switch_pressed_signal = xSemaphoreCreateBinary();

 
  // switch_queue = xQueueCreate(1, sizeof(switch_e));

  // binary_semaphore = xSemaphoreCreateBinary();
 
  // // Lab 3: Part 2
  // // LPC_GPIO0->DIR &= ~pin30;
  // xTaskCreate(led_task, "led", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pin30_isr);
  // gpio0__interrupt_dispatcher();

  // Lab 3: Set P0.30 as input [Lab 3] --
  // LPC_GPIO0->DIR &= ~pin30;
  // // Enable Interrupt
  // LPC_GPIOINT->IO0IntEnR = pin30;
  // lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, "name");

  // NVIC_EnableIRQ(GPIO_IRQn);

  // xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
 
  puts("Starting RTOS..hello world..hey");

}
