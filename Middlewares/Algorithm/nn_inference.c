#include "nn_inference.h"
#include "fall_model_weights.h"
#include <math.h>

static float relu(float x) {
    return (x > 0.0f) ? x : 0.0f;
}

static float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

float NN_Inference(const float features[NN_INPUT_SIZE]) {
    float x[NN_INPUT_SIZE];
    float h1[NN_HIDDEN1_SIZE];
    float h2[NN_HIDDEN2_SIZE];
    float output;
    int i, j;

    /* Normalize input features */
    for (i = 0; i < NN_INPUT_SIZE; i++) {
        x[i] = (features[i] - FEAT_MEAN[i]) / FEAT_STD[i];
    }

    /* Layer 1: 16 -> 16, ReLU */
    for (j = 0; j < NN_HIDDEN1_SIZE; j++) {
        float sum = b1[j];
        for (i = 0; i < NN_INPUT_SIZE; i++) {
            sum += x[i] * W1[j * NN_INPUT_SIZE + i];
        }
        h1[j] = relu(sum);
    }

    /* Layer 2: 16 -> 8, ReLU */
    for (j = 0; j < NN_HIDDEN2_SIZE; j++) {
        float sum = b2[j];
        for (i = 0; i < NN_HIDDEN1_SIZE; i++) {
            sum += h1[i] * W2[j * NN_HIDDEN1_SIZE + i];
        }
        h2[j] = relu(sum);
    }

    /* Layer 3: 8 -> 1, Sigmoid */
    output = b3[0];
    for (i = 0; i < NN_HIDDEN2_SIZE; i++) {
        output += h2[i] * W3[i];
    }
    output = sigmoid(output);

    return output;
}
