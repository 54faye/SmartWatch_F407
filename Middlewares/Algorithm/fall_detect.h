#ifndef __FALL_DETECT_H
#define __FALL_DETECT_H

#include <stdint.h>

/* Configuration */
#define FALL_WINDOW_SIZE         50
#define FALL_PRE_FILTER_THRESH   0.8f
#define FALL_PROB_THRESHOLD      0.5f
#define FALL_CONFIRM_COUNT       2
#define FALL_COOLDOWN_MS         10000
#define FALL_ALERT_DURATION_MS   5000

/* Feature vector: 16 dimensions for NN input */
typedef struct {
    float mean;
    float std;
    float max;
    float min;
    float range;
    float zcr;
    float sma;
    float peak_count;
    float peak_amplitude;
    float acc_diff;
    float energy;
    float roll_var;
    float pitch_var;
    float ax_mean;
    float ay_mean;
    float az_mean;
} FallFeatures_t;

/* Fall detection state machine */
typedef struct {
    int16_t buf_ax[FALL_WINDOW_SIZE];
    int16_t buf_ay[FALL_WINDOW_SIZE];
    int16_t buf_az[FALL_WINDOW_SIZE];
    uint8_t buf_index;
    uint8_t buf_full;

    float prev_accel_mag;
    uint8_t confirm_counter;
    uint32_t last_alert_tick;
    uint8_t alert_active;

    FallFeatures_t last_features;
    float last_probability;
} FallDetect_t;

/* Public API */
void FallDetect_Init(FallDetect_t *fd);
void FallDetect_Update(FallDetect_t *fd, int16_t ax, int16_t ay, int16_t az,
                       float roll, float pitch, uint32_t tick_ms);
uint8_t FallDetect_IsAlerting(const FallDetect_t *fd);
float FallDetect_GetProbability(const FallDetect_t *fd);

#endif /* __FALL_DETECT_H */
