/*
 * This file is part of MobilECG, an open source clinical grade Holter
 * ECG. For more information visit http://mobilecg.hu
 *
 * Copyright (C) 2016  Robert Csordas, Peter Isza
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef ANDROIDAPP_ECGPROCESSOR_H
#define ANDROIDAPP_ECGPROCESSOR_H

#include "../res/Common/SignalConditioning/EcgFilters.hpp"
#include "../../../../../../AppData/Local/Android/Sdk/ndk-bundle/sources/cxx-stl/llvm-libc++/libcxx/include/ctime"

#define PULSE_THRESHOLD 0.5 // Pulse detection threshold
// Number of beats required for initial pulse detection.
#define PULSE_INITIAL_BEATS 3
// Pulse reset threshold.
#define PULSE_RESET_THRESHOLD 500.0
// Pulse timeout (in ms). If no pulse detected for this time, reset readings
#define PULSE_RESET_TIMEOUT 5000

class EcgProcessor {
    private:
        EcgProcessor();
        ~EcgProcessor();
        static const int MAX_NUM_CHANNELS = 8;
        EcgFilter ecgFilter[MAX_NUM_CHANNELS];

        int pga;
        float currScale;
        float samplingFrequency;
        int pulseCurrentBPM;

        // Pulse detection state.
        enum {
            PULSE_IDLE = 0,
            PULSE_FALLING = 1,
            PULSE_RISING = 2,
        };

    public:
        float getSamplingFrequency();
        static EcgProcessor &instance();
        static void calculate12Channels(float* input, float* output, int stride = 1);

        void receivePacket(char *data, int len);
        void writeDebugDataToFile(float *inputBefore, float *inputAfter);
        static int getCurrentTime();
        static int calculateBPM(float input);
};


#endif //ANDROIDAPP_ECGPROCESSOR_H
