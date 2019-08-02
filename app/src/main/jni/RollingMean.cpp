//
// Created by vidra on 2. 08. 2019.
//

#include "RollingMean.h"
#include <string.h>

RollingMean::RollingMean() {
    index = 0;
    sum = 0.0;
    count = 0;
}

float RollingMean::filterData(float x, int difference) {
    float avg = 0;

    sum -= values[index];
    values[index] = x;
    sum += values[index];

    index++;
    index = index % MEAN_FILTER_SIZE;

    if (count < MEAN_FILTER_SIZE) {
        count++;
    }

    avg = sum / count;
    if (difference) {
        // Difference from mean.
        return avg - x;
    }
    else {
        // Rolling mean.
        return avg;
    }
}

void RollingMean::resetFilter() {
    index = 0;
    sum = 0.0;
    count = 0;
    memset(values, 0, sizeof(values));
}