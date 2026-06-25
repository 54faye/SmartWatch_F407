#ifndef __NN_INFERENCE_H
#define __NN_INFERENCE_H

#include "fall_detect.h"

#define NN_INPUT_SIZE   16
#define NN_HIDDEN1_SIZE 16
#define NN_HIDDEN2_SIZE 8
#define NN_OUTPUT_SIZE  1

float NN_Inference(const float features[NN_INPUT_SIZE]);

#endif /* __NN_INFERENCE_H */
