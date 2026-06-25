#include "task_common.h"
#include "app_config.h"
#include "UI_Manager.h"
#include "bsp_key.h"

/**
  * @brief  ������������ - ���������¼����л�ҳ��
  * @param  argument: δʹ��
  * @retval None
  */
void KeyProcessTask(void *argument)
{
    uint8_t KeyNum = 0;
    
    while(1)
    {
        /* �����ȴ�������Ϣ����ʱ100ms */
        if(xQueueReceive(KeyQueue, &KeyNum, pdMS_TO_TICKS(100)) == pdPASS)
        {
            /* ������Ļ */
            IsScreenOn = 1;
            LastActionTime = xTaskGetTickCount();

            if (FallAlertActive) {
                FallAlertActive = 0;
            }

            /* ����UI�������������� */
            UI_HandleKey(KeyNum);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms��ʱ
    }
}

/**
  * @brief  ���Ͱ�����Ϣ������
  * @param  KeyNum: ����ֵ
  * @retval None
  */
void SendKeyToQueue(uint8_t KeyNum)
{
    if(KeyQueue != NULL)
    {
        xQueueSendFromISR(KeyQueue, &KeyNum, NULL);
    }
}
