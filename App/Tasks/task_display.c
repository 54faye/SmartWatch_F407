#include "task_common.h"
#include "app_config.h"
#include "UI_Manager.h"
#include "bsp_oled.h"
#include "bsp_led.h"
#include "debug_serial.h"
#include "fall_detect.h"

/* Power management globals */
volatile TickType_t LastActionTime = 0;
volatile uint8_t IsScreenOn = 1;

/**
  * @brief  DisplayTask - OLED screen refresh
  * @param  argument: unused
  * @retval None
  */
void DisplayTask(void *argument)
{
    printf("[DBG] DisplayTask running\r\n");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(DISPLAY_REFRESH_PERIOD);
    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        /* Auto screen-off */
        if (IsScreenOn && (xTaskGetTickCount() - LastActionTime > pdMS_TO_TICKS(OLED_AUTO_OFF_TIMEOUT * 1000))) {
            IsScreenOn = 0;
            if(xSemaphoreTake(OLEDMutex, portMAX_DELAY) == pdTRUE) {
                OLED_Clear();
                OLED_Update();
                xSemaphoreGive(OLEDMutex);
            }
        }

        if (!IsScreenOn) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        /* Acquire OLED mutex */
        if(xSemaphoreTake(OLEDMutex, portMAX_DELAY) == pdTRUE)
        {
            /* Fall alert: takeover entire display */
            if (FallAlertActive) {
                OLED_Clear();
                OLED_ShowString(0, 0, "FALL DETECTED!", OLED_8X16);
                OLED_ShowString(0, 2, "Press key to clear", OLED_8X16);
            } else {
                UI_Refresh();
            }

            OLED_Update();
            xSemaphoreGive(OLEDMutex);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
