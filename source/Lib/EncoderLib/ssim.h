#pragma once

#include <stdint.h>

double CalculateSSIM(const uint8_t *px, unsigned stride_x, const uint8_t *py, unsigned stride_y, unsigned width, unsigned height,
    unsigned sample_max);
double CalculateSSIM(const int16_t *px, unsigned stride_x, const int16_t *py, unsigned stride_y, unsigned width, unsigned height,
    unsigned sample_max);