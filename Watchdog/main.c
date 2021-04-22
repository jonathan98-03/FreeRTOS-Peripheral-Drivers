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

TaskHandle_t consumer_handle;
QueueHandle_t sensor_queue;
EventGroupHandle_t watchdog_task;

const uint32_t cons_task = (1 << 2);
const uint32_t prod_task = (1 << 1);

int get_acceleration_data() {
  acceleration__axis_data_s axis_data;

  if (sensors__init()) {
    axis_data = acceleration__get_data();
  }

  return axis_data.y;
}

int get_average() {
  int samples_of_100[100];
  int i = 0;
  int sum = 0;
  int avg = 0;

  while (i < 100) {
    // printf("Tick count:%dms \n", xTaskGetTickCount());
    // printf("Sample size: %d \n", i);
    samples_of_100[i] = get_acceleration_data();
    // printf("Value: %d \n", samples_of_100[i]);
    sum += samples_of_100[i];
    // printf("Sum: %d \n", sum);
    i++;
  }

  avg = sum / 100;
  // printf("Avg: %d \n", avg);
  return avg;
}

void write_file(int avg) {
  const char *filename = "sensor_file.txt";
  FIL file;
  UINT bytes_written = 0;
  // FRESULT result = f_open(&file, filename, (FA_WRITE | FA_CREATE_ALWAYS));
  // FRESULT result = f_open(&file, filename, (FA_WRITE | FA_OPEN_ALWAYS));
  // FRESULT result = f_open(&file, filename, (0x02 | 0x30));
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_OPEN_APPEND));

  if (FR_OK == result) {
    char string[64];
    sprintf(string, "%i, %i\n", avg, xTaskGetTickCount());
    printf("This is String: %s \n", string);
    vTaskDelay(500);
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
      printf("Success in writing file\n");
    } else {
      printf("Error: Failed to write data to file\n");
    }
    // f_close(&file);
    f_sync(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}

void producer_task(void *p) {
  int avg = 0;
  // avg = get_average();
  // printf("About to Send: \n");
  // vTaskSuspend(consumer_handle);
  while (1) {
    avg = get_average();
    if (xQueueSend(sensor_queue, &avg, 0)) {
      // avg = get_average();
    }

    xEventGroupSetBits(watchdog_task, prod_task);
    vTaskDelay(100);
  }
}

void consumer_task(void *p) {
  int avg;
  // printf("About to Recieve \n");
  int time = xTaskGetTickCount();
  while (1) {
    xQueueReceive(sensor_queue, &avg, portMAX_DELAY);
    write_file(avg);
    xEventGroupSetBits(watchdog_task, cons_task);
  }
}

void task_watchdog(void *p) {
  const uint32_t task_all_bits = prod_task | cons_task;
  while (1) {
    uint32_t result = xEventGroupWaitBits(watchdog_task, task_all_bits, pdTRUE, pdTRUE, 1000);

    if ((result & task_all_bits) == task_all_bits) {
      printf("Succesfully Verify check in of other tasks \n");
    } else {
      if (!(result & prod_task)) {
        printf("Producer Task Stopped Responding \n");
      }

      if (!(result & cons_task)) {
        printf("Consumer Task Stopped Responding \n");
      }
    }
  }
}

void main(void) {

  // Lab 8
  // sensor_queue = xQueueCreate(1, sizeof(int));
  // watchdog_task = xEventGroupCreate();
  // xTaskCreate(producer_task, "producer", (512U * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(consumer_task, "consumer", (512U * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, &consumer_handle);
  // xTaskCreate(task_watchdog, "watchdog", (512U * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);



  puts("Starting RTOS..hello world..hey");

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

}
