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

//[Lab 4] --
void pwm_task(void *p) {
  pwm1__init_single_edge(20);
  // Locate a GPIO pin that a PWM channel will control
  // NOTE You can use gpio__construct_with_function() API from gpio.h
  // TODO Write this function yourself
  gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCTION_1);

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%
  pwm1__set_duty_cycle(PWM1__2_0, 50);

  // Continue to vary the duty cycle in the loop
  uint8_t percent = 0;
  int adc_reading = 0;
  while (1) {
    // pwm1__set_duty_cycle(PWM1__2_0, percent);

    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) {
      printf("\n\nReceived acd_reading: %d \n", adc_reading);
      float adc_reading2 = (float)adc_reading;
      float percentage = (adc_reading2 / 4095.0) * 100;
      printf("Received value in percentage: %f \n", percentage);
      printf("\nMatch Register 0: %d \nMatch Register 1: %d \n", LPC_PWM1->MR0, LPC_PWM1->MR1);
      pwm1__set_duty_cycle(PWM1__2_0, percentage);
    }

    // if (++percent > 100) {
    //   percent = 0;
    // }

    // vTaskDelay(100);
  }
}

// Part 1
void adc_task(void *p) {
  // adc__initialize();
  LPC_IOCON->P1_31 &= ~(1 << 7); // Enable Analog mode
  // LPC_GPIO1->DIR &= ~(1 << 31);  // Set P1.31 as input
  LPC_IOCON->P1_31 &= ~0b111;
  LPC_IOCON->P1_31 |= 0b011;                  // select function 3, P1.31 = ADC[5]
  LPC_IOCON->P1_31 &= ~((1 << 3) | (1 << 4)); // MODE field should be disabled
  adc__initialize();

  // TODO This is the function you need to add to adc.h
  // You can configure burst mode for just the channel you are using
  adc__enable_burst_mode();

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  // LPC_IOCON->P1_31 |= 0b011;

  int adc_reading = 0;
  while (1) {
    // Get the ADC reading using a new routine you created to read an ADC burst reading
    // TODO: You need to write the implementation of this function
    const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(5);
    // const uint16_t adc_value = adc__get_adc_value(ADC__CHANNEL_5);
    const uint16_t result = adc_value;
    float dc_value = ((float)adc_value / 4095) * 3.3;
    adc_reading = adc_value;

    if (!xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0)) {
      puts("Error: unable to send item");
    } else {
      printf("\n\nSuccessfully sent %u \n", adc_reading);
    }

    printf("Potentiometer value: %u \n", result);
    printf("Voltage value: %f \n", dc_value);

    vTaskDelay(1000);
  }
}

void pwm_task_test(void *p) {
  pwm1__init_single_edge(1000);
  uint8_t duty_percent = 10;
  gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCTION_1);
  while (1) {
    pwm1__set_duty_cycle(PWM1__2_0, duty_percent);
    printf("Duty cycle percent:%i \n", duty_percent);
    vTaskDelay(100);
  }
}

void mp3_player_task(void *p) {
  char data_bytes[512];

  while (1) {
    fprintf(stderr, "About to receive \n");
    xQueueReceive(Q_songdata, &data_bytes[0], portMAX_DELAY);
    fprintf(stderr, "Received \n");
    for (int i = 0; i < sizeof(data_bytes); i++) {
      // putchar(data_bytes[i]);
      fprintf(stderr, "Index %d: %c \n", i, data_bytes[i]);
    }
  }
}

void main(void) {
  // Queue will only hold 1 integer
  // adc_to_pwm_task_queue = xQueueCreate(1, sizeof(uint16_t));


  // Lab 4 [Lab 4] --
  // xTaskCreate(pwm_task, "pwm", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  // // Lab 4: Part 1
  // xTaskCreate(adc_task, "adc", (512U * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  // xTaskCreate(pwm_task_test, "test", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);




  puts("Starting RTOS..hello world..hey");

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

}
