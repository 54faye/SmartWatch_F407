#include "task_common.h"
#include "app_config.h"
#include "DataCenter.h"
#include "UI_Manager.h"
#include "debug_serial.h"
#include "test_ota.h"
#include <stdio.h>

/* Task handles */
TaskHandle_t StartTaskHandle = NULL;
TaskHandle_t DisplayTaskHandle = NULL;
TaskHandle_t KeyTaskHandle = NULL;
TaskHandle_t MPU6050TaskHandle = NULL;

/* Queue/Semaphore */
QueueHandle_t KeyQueue = NULL;
SemaphoreHandle_t OLEDMutex = NULL;

/* External task functions */
extern void DisplayTask(void *argument);
extern void KeyProcessTask(void *argument);
extern void MPU6050Task(void *argument);

/* StartTask - initializes everything then creates app tasks */
void StartTask(void *argument)
{
    printf("[DBG] StartTask running\r\n");

    DataCenter_Init();
    UI_Init();
    Serial_Init();
    printf("[DBG] DataCenter/UI/Serial OK\r\n");

    /* OTA测试入口 - 测试完成后删除或注释掉 */
    OTA_TestMenu();
    Test_W25Q128();
    Test_AT24C02();
    Test_ESP8266_AT();  /* 测试ESP8266 AT指令通信 */
    Test_ESP8266();     /* 测试WiFi连接 */
    Test_OTA_Full();    /* 完整OTA测试: 下载→W25Q128→校验→设置标志 */

    KeyQueue = xQueueCreate(QUEUE_LENGTH_KEY, QUEUE_ITEM_SIZE_KEY);
    if(KeyQueue == NULL) { while(1); }

    OLEDMutex = xSemaphoreCreateMutex();
    if(OLEDMutex == NULL) { while(1); }
    printf("[DBG] Queue/Mutex OK\r\n");

    xTaskCreate(DisplayTask, "DispTask", TASK_STACK_SIZE_DISPLAY, NULL, TASK_PRIORITY_DISPLAY, &DisplayTaskHandle);
    xTaskCreate(KeyProcessTask, "KeyTask", TASK_STACK_SIZE_KEY, NULL, TASK_PRIORITY_KEY, &KeyTaskHandle);
    xTaskCreate(MPU6050Task, "MPUTask", TASK_STACK_SIZE_MPU6050, NULL, TASK_PRIORITY_MPU6050, &MPU6050TaskHandle);
    printf("[DBG] All tasks created\r\n");
#if FALL_DETECT_ENABLE
    printf("[DBG] Fall Detection: ENABLED (pre-filter=%.2fg)\r\n", FALL_PRE_FILTER_THRESH);
#endif

    vTaskDelete(NULL);
}

/* Idle hook */
void vApplicationIdleHook(void)
{
    __WFI();
}

/* Stack overflow hook */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("[ERR] Stack overflow: %s\r\n", pcTaskName);
    for(;;);
}

/* Malloc failed hook */
void vApplicationMallocFailedHook(void)
{
    printf("[ERR] Malloc failed!\r\n");
    for(;;);
}

/* Static allocation - Idle task */
static StaticTask_t xIdleTaskTCB;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = xIdleStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* Static allocation - Timer task */
static StaticTask_t xTimerTaskTCB;
static StackType_t xTimerStack[80];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = xTimerStack;
    *pulTimerTaskStackSize = 80;
}
