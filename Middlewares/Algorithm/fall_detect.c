#include "fall_detect.h"
#include "nn_inference.h"
#include <math.h>
#include <string.h>

#define ACCEL_SENSITIVITY 2048.0f

static void extract_features(const FallDetect_t *fd, FallFeatures_t *f,
                             float roll, float pitch) {
    float mag[FALL_WINDOW_SIZE];
    float ax_f, ay_f, az_f;
    float sum = 0.0f, sq_sum = 0.0f;
    float max_val = -1e9f, min_val = 1e9f;
    float sma_sum = 0.0f;
    int i, start;
    int peak_count = 0;
    int zc_count = 0;
    float max_diff = 0.0f;

    start = fd->buf_full ? fd->buf_index : 0;
    int count = fd->buf_full ? FALL_WINDOW_SIZE : fd->buf_index;

    if (count < FALL_WINDOW_SIZE) {
        memset(f, 0, sizeof(FallFeatures_t));
        return;
    }

    /* Compute magnitude array and basic stats */
    for (i = 0; i < FALL_WINDOW_SIZE; i++) {
        int idx = (start + i) % FALL_WINDOW_SIZE;
        ax_f = (float)fd->buf_ax[idx] / ACCEL_SENSITIVITY;
        ay_f = (float)fd->buf_ay[idx] / ACCEL_SENSITIVITY;
        az_f = (float)fd->buf_az[idx] / ACCEL_SENSITIVITY;

        mag[i] = sqrtf(ax_f * ax_f + ay_f * ay_f + az_f * az_f);
        sum += mag[i];
        sq_sum += mag[i] * mag[i];

        if (mag[i] > max_val) max_val = mag[i];
        if (mag[i] < min_val) min_val = mag[i];

        sma_sum += (ax_f > 0.0f ? ax_f : -ax_f) + (ay_f > 0.0f ? ay_f : -ay_f) + (az_f > 0.0f ? az_f : -az_f);

        if (i > 0) {
            float diff = (mag[i] > mag[i - 1]) ? (mag[i] - mag[i - 1]) : (mag[i - 1] - mag[i]);
            if (diff > max_diff) max_diff = diff;
        }
    }

    float mean = sum / FALL_WINDOW_SIZE;
    float std = sqrtf(sq_sum / FALL_WINDOW_SIZE - mean * mean);
    if (std < 0.0f) std = 0.0f;

    /* Zero-crossing rate of (mag - mean) */
    float prev_sign = mag[0] - mean;
    for (i = 1; i < FALL_WINDOW_SIZE; i++) {
        float cur_sign = mag[i] - mean;
        if ((prev_sign >= 0.0f && cur_sign < 0.0f) ||
            (prev_sign < 0.0f && cur_sign >= 0.0f)) {
            zc_count++;
        }
        prev_sign = cur_sign;
    }
    float zcr = (float)zc_count / (FALL_WINDOW_SIZE - 1);

    /* Peak count: samples above 1.5g AND above mean + 1.5*std */
    float peak_thresh = mean + 1.5f * std;
    if (peak_thresh < 1.5f) peak_thresh = 1.5f;
    for (i = 0; i < FALL_WINDOW_SIZE; i++) {
        if (mag[i] > peak_thresh) peak_count++;
    }

    /* Axis means */
    float sum_ax = 0.0f, sum_ay = 0.0f, sum_az = 0.0f;
    for (i = 0; i < FALL_WINDOW_SIZE; i++) {
        int idx = (start + i) % FALL_WINDOW_SIZE;
        sum_ax += fd->buf_ax[idx];
        sum_ay += fd->buf_ay[idx];
        sum_az += fd->buf_az[idx];
    }

    f->mean = mean;
    f->std = std;
    f->max = max_val;
    f->min = min_val;
    f->range = max_val - min_val;
    f->zcr = zcr;
    f->sma = sma_sum / FALL_WINDOW_SIZE;
    f->peak_count = (float)peak_count;
    f->peak_amplitude = max_val - min_val;
    f->acc_diff = max_diff;
    f->energy = sq_sum / FALL_WINDOW_SIZE;
    f->roll_var = roll;
    f->pitch_var = pitch;
    f->ax_mean = sum_ax / (FALL_WINDOW_SIZE * ACCEL_SENSITIVITY);
    f->ay_mean = sum_ay / (FALL_WINDOW_SIZE * ACCEL_SENSITIVITY);
    f->az_mean = sum_az / (FALL_WINDOW_SIZE * ACCEL_SENSITIVITY);
}

void FallDetect_Init(FallDetect_t *fd) {
    memset(fd, 0, sizeof(FallDetect_t));
    fd->prev_accel_mag = -1.0f;
}

void FallDetect_Update(FallDetect_t *fd, int16_t ax, int16_t ay, int16_t az,
                       float roll, float pitch, uint32_t tick_ms) {

    /* Write sample into ring buffer */
    fd->buf_ax[fd->buf_index] = ax;
    fd->buf_ay[fd->buf_index] = ay;
    fd->buf_az[fd->buf_index] = az;
    fd->buf_index++;
    if (fd->buf_index >= FALL_WINDOW_SIZE) {
        fd->buf_index = 0;
        fd->buf_full = 1;
    }

    /* Compute acceleration magnitude in g */
    float ax_g = (float)ax / ACCEL_SENSITIVITY;
    float ay_g = (float)ay / ACCEL_SENSITIVITY;
    float az_g = (float)az / ACCEL_SENSITIVITY;
    float accel_mag = sqrtf(ax_g * ax_g + ay_g * ay_g + az_g * az_g);

    /* Pre-filter: acceleration differential */
    float acc_diff = 0.0f;
    if (fd->prev_accel_mag >= 0.0f) {
        acc_diff = (accel_mag > fd->prev_accel_mag) ? (accel_mag - fd->prev_accel_mag) : (fd->prev_accel_mag - accel_mag);
    }
    fd->prev_accel_mag = accel_mag;

    /* Alert cooldown management */
    if (fd->alert_active) {
        if ((tick_ms - fd->last_alert_tick) > FALL_ALERT_DURATION_MS) {
            fd->alert_active = 0;
        }
    }

    /* Only run NN inference when pre-filter triggers AND buffer is full */
    if (acc_diff > FALL_PRE_FILTER_THRESH && fd->buf_full) {
        FallFeatures_t feats;
        extract_features(fd, &feats, roll, pitch);

        float features_arr[NN_INPUT_SIZE];
        features_arr[0]  = feats.mean;
        features_arr[1]  = feats.std;
        features_arr[2]  = feats.max;
        features_arr[3]  = feats.min;
        features_arr[4]  = feats.range;
        features_arr[5]  = feats.zcr;
        features_arr[6]  = feats.sma;
        features_arr[7]  = feats.peak_count;
        features_arr[8]  = feats.peak_amplitude;
        features_arr[9]  = feats.acc_diff;
        features_arr[10] = feats.energy;
        features_arr[11] = feats.roll_var;
        features_arr[12] = feats.pitch_var;
        features_arr[13] = feats.ax_mean;
        features_arr[14] = feats.ay_mean;
        features_arr[15] = feats.az_mean;

        float prob = NN_Inference(features_arr);
        fd->last_probability = prob;
        fd->last_features = feats;

        if (prob > FALL_PROB_THRESHOLD) {
            fd->confirm_counter++;
            if (fd->confirm_counter >= FALL_CONFIRM_COUNT) {
                /* Check cooldown */
                if (fd->last_alert_tick == 0 ||
                    (tick_ms - fd->last_alert_tick) > FALL_COOLDOWN_MS) {
                    fd->alert_active = 1;
                    fd->last_alert_tick = tick_ms;
                }
                fd->confirm_counter = 0;
            }
        } else {
            fd->confirm_counter = 0;
        }
    }
}

uint8_t FallDetect_IsAlerting(const FallDetect_t *fd) {
    return fd->alert_active;
}

float FallDetect_GetProbability(const FallDetect_t *fd) {
    return fd->last_probability;
}
