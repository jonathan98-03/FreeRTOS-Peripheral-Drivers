#include "gpio_lab.h"

#include "lpc40xx.h"

void gpioN__set_as_input(uint8_t pin_num, uint8_t port_num) {

  if (port_num == 0) {
    LPC_GPIO0->DIR &= ~(1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->DIR &= ~(1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->DIR &= ~(1 << pin_num);
  }
}

void gpioN__set_as_output(uint8_t pin_num, uint8_t port_num) {

  if (port_num == 0) {
    LPC_GPIO0->DIR |= (1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->DIR |= (1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->DIR |= (1 << pin_num);
  }
}

void gpioN__set_high(uint8_t pin_num, uint8_t port_num) {

  if (port_num == 0) {
    LPC_GPIO0->SET = (1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->SET = (1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->SET = (1 << pin_num);
  }
}

void gpioN__set_low(uint8_t pin_num, uint8_t port_num) {
  if (port_num == 0) {
    LPC_GPIO0->CLR = (1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->CLR = (1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->CLR = (1 << pin_num);
  }
}

void gpioN__set(uint8_t pin_num, uint8_t port_num, bool high) {
  if (high == true) {

    if (port_num == 0) {
      LPC_GPIO0->SET = (1 << pin_num);
    } else if (port_num == 1) {
      LPC_GPIO1->SET = (1 << pin_num);
    } else if (port_num == 2) {
      LPC_GPIO2->SET = (1 << pin_num);
    }

  } else {
    if (port_num == 0) {
      LPC_GPIO0->CLR = (1 << pin_num);
    } else if (port_num == 1) {
      LPC_GPIO1->CLR = (1 << pin_num);
    } else if (port_num == 2) {
      LPC_GPIO2->CLR = (1 << pin_num);
    }
  }
}

bool gpioN__get_level(uint8_t pin_num, uint8_t port_num) {

  if (port_num == 0) {
    if (LPC_GPIO0->PIN & (1 << pin_num)) {
      return true;
    } else {
      return false;
    }
  } else if (port_num == 1) {
    if (LPC_GPIO1->PIN & (1 << pin_num)) {
      return true;
    } else {
      return false;
    }
  } else if (port_num == 2) {
    if (LPC_GPIO2->PIN & (1 << pin_num)) {
      return true;
    } else {
      return false;
    }
  }
}
