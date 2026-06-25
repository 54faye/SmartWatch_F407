/**
  ******************************************************************************
  * @file           : app_config.h
  * @brief          : Ӧ�������ļ�
  * @details        : �����������ȼ�����ջ��С�����г��ȵ�ϵͳ����
  ******************************************************************************
  */

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* ============================================================================ */
/*                            �������ȼ�����                                      */
/* ============================================================================ */
/* ���ȼ���Χ: 0-7 (configMAX_PRIORITIES-1)
 * ��ֵԽ�����ȼ�Խ��
 * �����������ȼ�Ϊ0
 */
#define TASK_PRIORITY_START         4    /* ��ʼ�������ȼ������𴴽��������� */
#define TASK_PRIORITY_KEY           3    /* �����������ȼ�����Ҫ��ʱ��Ӧ */
#define TASK_PRIORITY_DISPLAY       2    /* ��ʾ�������ȼ� */
#define TASK_PRIORITY_MPU6050       1    /* MPU6050�������ȼ������ݲɼ� */

/* ============================================================================ */
/*                            �����ջ��С����                                      */
/* ============================================================================ */
/* ��ջ��С��λ: Word (4�ֽ�)
 * STM32F103C8T6��RAM: 20KB
 * FreeRTOS�ں�ռ��: Լ6KB
 * �û����ö�ջ: Լ14KB
 */
#define TASK_STACK_SIZE_START       512  /* StartTask栈大小(含OTA下载+CRC) */
#define TASK_STACK_SIZE_DISPLAY     300  /* ��ʾ�����ջ��С��OLED_Printf��Ҫ�ϴ�ջ */
#define TASK_STACK_SIZE_KEY         90   /* ���������ջ��С */
#define TASK_STACK_SIZE_MPU6050     256  /* MPU6050�����ջ��С���漰�������� */

/* ============================================================================ */
/*                            ��������                                    */
/* ============================================================================ */
#define QUEUE_LENGTH_KEY            10   /* �������г��� */
#define QUEUE_ITEM_SIZE_KEY         sizeof(uint8_t)  /* ����������Ŀ��С */

/* ============================================================================ */
/*                            ��ʾˢ������                                      */
/* ============================================================================ */
/* ˢ�����ڵ�λ: ���� */
#define DISPLAY_REFRESH_PERIOD      50   /* ��ʾˢ������ 50ms (20fps) */

/* ��ͬ�����ˢ�����ڶ��� */
#define DISPLAY_REFRESH_CLOCK       1000 /* ʱ�ӽ���ˢ�� 1000ms (1fps) */
#define DISPLAY_REFRESH_MENU        30   /* �˵�����ˢ�� 30ms (33fps����֤����) */
#define DISPLAY_REFRESH_GAME        50   /* ��Ϸ����ˢ�� 50ms (20fps) */
#define DISPLAY_REFRESH_EMOJI       100  /* �������ˢ�� 100ms (10fps) */
#define DISPLAY_REFRESH_MPU6050     100  /* MPU6050����ˢ�� 100ms (10fps) */
#define DISPLAY_REFRESH_DEFAULT     200  /* Ĭ��ˢ������ 200ms (5fps) */

/* ============================================================================ */
/*                            MPU6050����                                    */
/* ============================================================================ */
#define FALL_DETECT_ENABLE          1    /* Fall detection: 1=on, 0=off */
#define MPU6050_SAMPLE_PERIOD_FAST  5    /* MPU6050���ٲ������� 5ms (200Hz) */
#define MPU6050_SAMPLE_PERIOD_SLOW  100

/* ============================================================================ */
/*                            Fall Detection Config                              */
/* ============================================================================ */
#define FALL_WINDOW_SIZE            50   /* Sliding window samples */
#define FALL_PRE_FILTER_THRESH      0.8f /* Accel diff threshold (g) */
#define FALL_PROB_THRESHOLD         0.5f /* NN probability threshold */
#define FALL_CONFIRM_WINDOWS        2    /* Consecutive windows to confirm */
#define FALL_COOLDOWN_SEC           10   /* Minimum seconds between alerts */
#define FALL_ALERT_DURATION_SEC     5    /* Alert LED blink duration (sec) */  /* MPU6050���ٲ������� 100ms (10Hz) */

/* ============================================================================ */
/*                            ��Դ��������                                         */
/* ============================================================================ */
#define ENABLE_TICKLESS_IDLE        1    /* ����Tickless����ģʽ��ʡ�� */
#define OLED_AUTO_OFF_TIMEOUT       180   /* OLED�Զ�Ϩ����ʱ (��) 0=��Ϩ�� */
#define OLED_AUTO_DIM_TIMEOUT       30   /* OLED�Զ��䰵��ʱ (��) 0=���䰵 */

/* ============================================================================ */
/*                            ̧�ֻ�������                                       */
/* ============================================================================ */
#define WAKE_UP_ANGLE_ROLL_MIN      -45  /* ���ѽǶ� Roll ��Сֵ */
#define WAKE_UP_ANGLE_ROLL_MAX      45   /* ���ѽǶ� Roll ���ֵ */
#define WAKE_UP_ANGLE_PITCH_MIN     -45  /* ���ѽǶ� Pitch ��Сֵ */
#define WAKE_UP_ANGLE_PITCH_MAX     10   /* ���ѽǶ� Pitch ���ֵ */
#define MOVEMENT_THRESHOLD          5.0f /* �˶���ֵ (��⶯������) */

/* ============================================================================ */
/*                            ��������                                           */
/* ============================================================================ */
#define ENABLE_RUNTIME_STATS        1    /* ��������ʱ��ͳ�� */
#define ENABLE_STACK_CHECK          1    /* ���ö�ջ������ */
#define ENABLE_MALLOC_CHECK         1    /* �����ڴ������ */

/* �ڴ澯����ֵ */
#define HEAP_WARNING_THRESHOLD      1024 /* ���ڴ澯����ֵ (�ֽ�) */
#define STACK_WARNING_THRESHOLD     20   /* ��ջ������ֵ (��) */

/* ============================================================================ */
/*                            ��������                                           */
/* ============================================================================ */
#define KEY_SCAN_PERIOD             1    /* ����ɨ������ (TIM2�ж�����ms) */
#define KEY_DEBOUNCE_COUNT          20   /* ������������ (20ms) */

/* ============================================================================ */
/*                            �������                                           */
/* ============================================================================ */
#define STOPWATCH_TICK_PERIOD       1    /* ����δ����� (ms) */

/* ============================================================================ */
/*                            ��Ϸ����                                           */
/* ============================================================================ */
#define GAME_UPDATE_PERIOD          20   /* ��Ϸ�߼��������� (ms) */
#define GAME_SCORE_UPDATE_PERIOD    100  /* ��Ϸ������������ (ms) */

/* ============================================================================ */
/*                            �ڴ�У��                                        */
/* ============================================================================ */
/* �����������ջ��С */
#define TOTAL_TASK_STACK_SIZE  (TASK_STACK_SIZE_START + \
                                TASK_STACK_SIZE_DISPLAY + \
                                TASK_STACK_SIZE_KEY + \
                                TASK_STACK_SIZE_MPU6050 + \
                                configMINIMAL_STACK_SIZE + \
                                configTIMER_TASK_STACK_DEPTH)

/* �������ڴ�ռ�� */
#define ESTIMATED_TOTAL_MEMORY  (TOTAL_TASK_STACK_SIZE * 4 + configTOTAL_HEAP_SIZE)

/* �ڴ��龯�� */
/* ��������ڴ泬�������ڴ棬�������ᷢ������ */
/* #if (ESTIMATED_TOTAL_MEMORY > 18432) */
/*     #warning "�������ڴ泬���˿����ڴ� (18KB)" */
/* #endif */

/* ============================================================================ */
/*                            �汾��Ϣ                                           */
/* ============================================================================ */
#define APP_VERSION_MAJOR       1
#define APP_VERSION_MINOR       0
#define APP_VERSION_PATCH       0
#define APP_VERSION_STRING      "V1.0.0"
#define APP_BUILD_DATE          __DATE__
#define APP_BUILD_TIME          __TIME__

#endif /* __APP_CONFIG_H */
