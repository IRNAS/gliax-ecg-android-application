//
// Created by vidra on 2. 08. 2019.
//

#ifndef ANDROIDAPP_ROLLINGMEAN_H
#define ANDROIDAPP_ROLLINGMEAN_H

// Mean filter window size
#define MEAN_FILTER_SIZE 15

class RollingMean {
    private:
        float values[MEAN_FILTER_SIZE];
        int index;
        float sum;
        int count;

    public:
        RollingMean();
        float filterData( float x, int difference);
        void resetFilter();
};

#endif //ANDROIDAPP_ROLLINGMEAN_H
