#ifndef __TASK_COMMON_H
#define __TASK_COMMON_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ������ */
extern TaskHandle_t StartTaskHandle;
extern TaskHandle_t DisplayTaskHandle;
extern TaskHandle_t KeyTaskHandle;
extern TaskHandle_t MPU6050TaskHandle;

/* ���о�� */
extern QueueHandle_t KeyQueue;

/* �����ź��� */
extern SemaphoreHandle_t OLEDMutex;

/* ȫ�ֱ�־λ */
extern uint8_t MPU6050_Available;
extern volatile uint8_t IsScreenOn;
extern volatile TickType_t LastActionTime;
extern volatile uint8_t FallAlertActive;

/* �ⲿ�ӿں��� */
void SendKeyToQueue(uint8_t KeyNum);

#endif
