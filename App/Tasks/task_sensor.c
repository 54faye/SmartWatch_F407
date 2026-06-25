#include "task_common.h"
#include "app_config.h"
#include "DataCenter.h"
#include "bsp_mpu6050.h"
#include "kalman.h"
#include "step_algo.h"
#include "UI_Manager.h"
#include "fall_detect.h"
#include "nn_inference.h"
#include "bsp_led.h"
#include <math.h>

/* MPU6050״̬ */
uint8_t MPU6050_Available = 0;

/* �ֲ��������� */
static int16_t ax, ay, az, gx, gy, gz;
static float Roll, Pitch, Yaw;

/* �������˲����� */
static Kalman_t K_Roll;
static Kalman_t K_Pitch;

/* Fall detection */
volatile uint8_t FallAlertActive = 0;
static FallDetect_t fall_detect;

/**
  * @brief  MPU6050���� - ��ȡ���������ݲ�������̬
  * @param  argument: δʹ��
  * @retval None
  */
void MPU6050Task(void *argument)
{
    float roll_a, pitch_a;
    float Delta_t = 0.005f;
    const float pi = 3.14159265f;
    TickType_t samplePeriod = pdMS_TO_TICKS(MPU6050_SAMPLE_PERIOD_FAST);
    static float last_Roll = 0, last_Pitch = 0;
    static uint8_t last_posture_ok = 0; // ��¼��һ����̬�Ƿ���������
    
    /* ��ʼ���������˲��� */
    Kalman_Init(&K_Roll);
    Kalman_Init(&K_Pitch);
    FallDetect_Init(&fall_detect);

    Roll = 0;
    Pitch = 0;
    Yaw = 0;
    
    while(1)
    {
        if(MPU6050_Available) {
            /* MPU6050���ã���ȡ���� */
            MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
            
            /* ���¼Ʋ��� */
            StepCounter_Update(ax, ay, az);
        } else {
            /* MPU6050�����ã�ʹ��Ĭ��ֵ */
            ax = ay = az = 0;
            gx = gy = gz = 0;
        }
        
        /* ������ٶȼƽǶ� */
        pitch_a = atan2f((-1) * ax, az) * 180.0f / pi;
        roll_a = atan2f((-1) * ay, az) * 180.0f / pi;
        
        /* �������˲��ں� (������������ 16.4 LSB/dps) */
        Roll = Kalman_GetAngle(&K_Roll, roll_a, gx / 16.4f, Delta_t);
        Pitch = Kalman_GetAngle(&K_Pitch, pitch_a, gy / 16.4f, Delta_t);
        Yaw += (gz / 16.4f) * Delta_t; // Yaw�򵥻���
        
        /* ������������ */
        DataCenter_SetSensorData(Roll, Pitch, Yaw);

        /* Fall detection */
        #if FALL_DETECT_ENABLE
        {
            uint32_t tick = xTaskGetTickCount();
            FallDetect_Update(&fall_detect, ax, ay, az, Roll, Pitch, tick);
            if (FallDetect_IsAlerting(&fall_detect)) {
                LED_RED_TOGGLE;
                LED_BLUE_TOGGLE;
                FallAlertActive = 1;
                IsScreenOn = 1;
                LastActionTime = tick;
            }
        }
        #endif
        
        /* �˶���� (�����Զ�Ϩ��) */
        if (fabs(Roll - last_Roll) > MOVEMENT_THRESHOLD || fabs(Pitch - last_Pitch) > MOVEMENT_THRESHOLD) {
            LastActionTime = xTaskGetTickCount();
        }
        last_Roll = Roll;
        last_Pitch = Pitch;
        
        /* ̧��������� (���ش���) */
        if (!IsScreenOn) {
            uint8_t current_posture_ok = 0;
            if (Roll >= WAKE_UP_ANGLE_ROLL_MIN && Roll <= WAKE_UP_ANGLE_ROLL_MAX &&
                Pitch >= WAKE_UP_ANGLE_PITCH_MIN && Pitch <= WAKE_UP_ANGLE_PITCH_MAX) {
                current_posture_ok = 1;
            }

            // ֻ�е���̬�ӡ������㡱��Ϊ�����㡱ʱ�Ŵ�������
            if (current_posture_ok && !last_posture_ok) {
                IsScreenOn = 1;
                LastActionTime = xTaskGetTickCount();
            }
            last_posture_ok = current_posture_ok;
        } else {
            // ��Ļ��ʱ��ʵʱ������̬״̬����ֹϨ��˲������
            if (Roll >= WAKE_UP_ANGLE_ROLL_MIN && Roll <= WAKE_UP_ANGLE_ROLL_MAX &&
                Pitch >= WAKE_UP_ANGLE_PITCH_MIN && Pitch <= WAKE_UP_ANGLE_PITCH_MAX) {
                last_posture_ok = 1;
            } else {
                last_posture_ok = 0;
            }
        }
        
        /* ���ݵ�ǰҳ�涯̬�����������Խ�ʡ���� */
        if(IsScreenOn && (UI_GetCurrentPage() == PAGE_MPU6050 || UI_GetCurrentPage() == PAGE_GRADIENTER))
        {
            samplePeriod = pdMS_TO_TICKS(MPU6050_SAMPLE_PERIOD_FAST);
        }
        else
        {
            samplePeriod = pdMS_TO_TICKS(MPU6050_SAMPLE_PERIOD_SLOW);
        }
        
        vTaskDelay(samplePeriod);
    }
}
