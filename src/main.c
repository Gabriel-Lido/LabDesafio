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

/***********************************LED THREAD*********************************/
void led_thread(void *arg)
{
  param_leds *led = (param_leds *)arg;
  uint8_t is_selected = 0;
  param_leds manager;
  int dutyCycle = 50;
  
  while (1)
  {
    
    if(led->current_led == LED1) {
      if(osMessageQueueGet(queue_led1 , &manager, NULL, 1) == osOK) {
        if(led->current_led == manager.current_led) {
          is_selected = 1;
          if(manager.increment_pwm) dutyCycle+=10;
        } else is_selected = 0;
      }
    } else if (led->current_led == LED2) {
      if(osMessageQueueGet(queue_led2 , &manager, NULL, 1) == osOK) {
        if(led->current_led == manager.current_led) {
          is_selected = 1;
          if(manager.increment_pwm) dutyCycle+=10;
        } else is_selected = 0;
      }
    } else if (led->current_led == LED3) {
      if(osMessageQueueGet(queue_led3 , &manager, NULL, 1) == osOK) {
        if(led->current_led == manager.current_led) {
          is_selected = 1;
          if(manager.increment_pwm) dutyCycle+=10;
        } else is_selected = 0;
      }
    } else if (led->current_led == LED4) {
      if(osMessageQueueGet(queue_led4 , &manager, NULL, 1) == osOK) {
        if(led->current_led == manager.current_led) {
          is_selected = 1;
          if(manager.increment_pwm) dutyCycle+=10;
        } else is_selected = 0;
      }
    }
    
    if(dutyCycle == 110) dutyCycle = 0;
    osMutexAcquire(mutex_id, osWaitForever);
    LEDOn(led->current_led);
    osMutexRelease(mutex_id);
    osDelay(10 * (dutyCycle / 100.f)); 
             
    if(is_selected) osDelay(200);
  
    osMutexAcquire(mutex_id, osWaitForever);
    LEDOff(led->current_led);
    osMutexRelease(mutex_id);
    osDelay(10 * (1.f - dutyCycle / 100.f));
    
    if(is_selected) osDelay(200);
  }
}

/*********************************MANAGER THREAD*********************************/
void manager_thread(void *arg){

  uint32_t flag;
  param_leds selected;
  
  selected.current_led = 1;
  
  while(1) {
    
   flag = osThreadFlagsWait(SW1_PRESSED | SW2_PRESSED , osFlagsWaitAny, osWaitForever);
   
    if(flag == SW1_PRESSED){
      selected.current_led = selected.current_led << 1;
      if(selected.current_led == 16) selected.current_led = 1;  
      
      selected.increment_pwm = 0;
      osMessageQueuePut(queue_led1 , &selected, NULL, 1);
      osMessageQueuePut(queue_led2 , &selected, NULL, 1);
      osMessageQueuePut(queue_led3 , &selected, NULL, 1);
      osMessageQueuePut(queue_led4 , &selected, NULL, 1);
      
    } else if (SW2_PRESSED) {
      selected.increment_pwm = 1;
      osMessageQueuePut(queue_led1 , &selected, NULL, 1);
      osMessageQueuePut(queue_led2 , &selected, NULL, 1);
      osMessageQueuePut(queue_led3 , &selected, NULL, 1);
      osMessageQueuePut(queue_led4 , &selected, NULL, 1);
    }
  }
}

/*************************************MAIN*************************************/
void main(void)
{
  param_leds leds[N_LEDS];
  SystemInit();
  LEDInit(LED4 | LED3 | LED2 | LED1);
  configButton();

  osKernelInitialize();

  leds[0].current_led = LED1;
  leds[1].current_led = LED2;
  leds[2].current_led = LED3;
  leds[3].current_led = LED4;
  
  mutex_id = osMutexNew(NULL);
    
  queue_led1 = osMessageQueueNew(N_LEDS , N_LEDS, NULL);
  queue_led2 = osMessageQueueNew(N_LEDS , N_LEDS, NULL);  
  queue_led3 = osMessageQueueNew(N_LEDS , N_LEDS, NULL);
  queue_led4 = osMessageQueueNew(N_LEDS , N_LEDS, NULL);

  managerThread = osThreadNew(manager_thread, NULL, NULL);
  ledThread1 = osThreadNew(led_thread, (param_leds *)&leds[0], NULL);
  ledThread2 = osThreadNew(led_thread, (param_leds *)&leds[1], NULL);  
  ledThread3 = osThreadNew(led_thread, (param_leds *)&leds[2], NULL);
  ledThread4 = osThreadNew(led_thread, (param_leds *)&leds[3], NULL);

  if (osKernelGetState() == osKernelReady)
    osKernelStart();

  while (1)
    ;
} // main

/*Inicialização dos botões*/
void configButton()
{
  ButtonInit(USW1 | USW2);
  ButtonIntEnable(USW1 | USW2);
}

/*Variaveis de Deboucing*/
const int debouncing_time = 250;
int last_Tick = 0;

/*Handler de Interrupcao*/
void button_ISR(void)
{
  int sw1 = 0, sw2 = 0;
  
  int getTick = osKernelGetTickCount();

  if(GPIOIntStatus(GPIO_PORTJ_BASE, true) & GPIO_PIN_0){
       sw1 = 1;
       GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
  }
  if(GPIOIntStatus(GPIO_PORTJ_BASE, true) & GPIO_PIN_1){
       sw2 = 1;
       GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_1);
  }
  
  if (getTick - last_Tick >= debouncing_time)
  {
    if(sw1) osThreadFlagsSet(managerThread, SW1_PRESSED);
    if(sw2) osThreadFlagsSet(managerThread, SW2_PRESSED);
    last_Tick = getTick;
  }
}
