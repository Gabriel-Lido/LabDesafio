#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h"      // device drivers
#include "driverbuttons.h"
#include "cmsis_os2.h"       // CMSIS-RTOS
#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

#define SW1_PRESSED  1
#define SW2_PRESSED  2
#define N_LEDS       4

/*Structs*/
typedef struct{
  uint8_t current_led;
  uint8_t increment_pwm;
}param_leds;

void configButton();

/*Threads*/
osThreadId_t managerThread, ledThread1, ledThread2, ledThread3, ledThread4;

/*Queues*/
osMessageQueueId_t queue_led1, queue_led2, queue_led3, queue_led4;

/*Mutex*/
osMutexId_t mutex_id;
