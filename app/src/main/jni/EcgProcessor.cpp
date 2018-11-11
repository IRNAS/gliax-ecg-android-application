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
 
#include "EcgProcessor.h"

#include <stdio.h>
#include <GLES2/gl2.h>
#include <time.h>

#include "EcgArea.h"
#include "log.h"
#include "../res/Common/DataFormat/EcgHeaderCommon.h"
#include "../res/Common/DataFormat/BitFifo.cpp"
#include "../res/Common/DataFormat/FlatEcgPredictor.cpp"
#include "../res/Common/DataFormat/DifferenceEcgCompressor.cpp"

//#define DEBUG
#define DEBUGFILE // uncomment this to write signal data to files

#define PULSE_THRESHOLD 0.5 // Pulse detection threshold
// Number of beats required for initial pulse detection.
#define PULSE_INITIAL_BEATS 3
// Pulse reset threshold.
#define PULSE_RESET_THRESHOLD 500.0
// Pulse timeout (in ms). If no pulse detected for this time, reset readings
#define PULSE_RESET_TIMEOUT 5000

const int DECOMPRESS_BUFFER_STRIDE = ECG_MAX_SEND_SIZE/3+1;
static GLfloat decompressBuffer[12][DECOMPRESS_BUFFER_STRIDE];
static GLfloat unFilteredBuffer[12][DECOMPRESS_BUFFER_STRIDE];

char filePath[64];
#ifdef DEBUGFILE
    FILE *af;
    FILE *bf;
#endif

char *getCurrentTime() {
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    char timeBuffer [80];
    strftime(timeBuffer, 80, "%H%M%S", localtime(&curTime.tv_sec));
    static char currentTime[84] = "";
    sprintf(currentTime, "%s%d", timeBuffer, milli);
    return currentTime;
}

int getClock() {
    timeval curTime;
    int milli = curTime.tv_usec / 1000;
    return milli;
}

// Pulse detection state.
enum {
    PULSE_IDLE = 0,
    PULSE_FALLING = 1,
    PULSE_RISING = 2,
};

uint8_t pulse_state = PULSE_IDLE;
clock_t pulse_current_timestamp = 0;
clock_t pulse_last_timestamp = 0;
uint8_t pulse_beats = 0;
uint8_t pulse_present = 0;
float pulse_previous_value = 0.0;
float pulse_current_bpm = 0.0;

EcgProcessor::EcgProcessor(){
    samplingFrequency=500.0;
    pulseCurrentBPM = 420;

    #ifdef DEBUGFILE
        strcpy(filePath, EcgArea::instance().internalStoragePath);
        strcat(filePath, "/after_filters.txt");
        LOGI("path: %s", filePath);
        if ((af = fopen(filePath, "w+")) == NULL) {
            LOGE("After filters: Error writing to file!");
        }
        fprintf(af, "timestamp,I,II,V1,V2,V3,V4,V5,V6\n");
        filePath[0] = 0;
        strcpy(filePath, EcgArea::instance().internalStoragePath);
        strcat(filePath, "/before_filters.txt");
        LOGI("path: %s", filePath);
        if ((bf = fopen(filePath, "w+")) == NULL) {
            LOGE("Before filters: Error writing to file!");
        }
        fprintf(bf, "timestamp,I,II,V1,V2,V3,V4,V5,V6\n");
    #endif
}

EcgProcessor::~EcgProcessor() {
    #ifdef DEBUGFILE
        fclose(af);
        fclose(bf);
    #endif
}

EcgProcessor &EcgProcessor::instance(){
    static EcgProcessor ecg;
    return ecg;
}

void EcgProcessor::receivePacket(char *data, int len){
    if (len<sizeof(ECGHeader))
        return;

    ECGHeader *header = (ECGHeader *)data;
    ecg::BitFifo bitFifo(data+sizeof(ECGHeader),(header->numBits+7)/8, header->numBits);
    ecg::FlatEcgPredictor predictor;
    predictor.setNumChannels(header->channelCount);
    predictor.reset();

    #ifdef DEBUG
        LOGD("numbits = %d, sampleCount = %d", header->numBits, header->sampleCount);
    #endif

    ecg::DifferenceEcgCompressor decompressor(bitFifo, predictor);
    decompressor.setNumChannels(header->channelCount);

    samplingFrequency=header->samplingFrequency;

    if(header->channelCount > MAX_NUM_CHANNELS) {
        LOGE("EcgProcessor max channel number exceeded.");
        return;
    }

    int32_t timesample [ecg::DifferenceEcgCompressor::maxChannels];

    int filteredSampleNum[MAX_NUM_CHANNELS];
    for (int c=0; c<header->channelCount; c++) {
        filteredSampleNum[c] = 0;
    }

    for (int a=0; a<header->sampleCount; a++){
        decompressor.getSample(timesample);

        #ifdef DEBUG
        if(a == 0)
            LOGD("%08X %08X %08X %08X %08X %08X %08X %08X", timesample[0], timesample[1], timesample[2], timesample[3], timesample[4], timesample[5], timesample[6], timesample[7]);
        #endif

        for (int c=0; c<header->channelCount; c++){
            decompressBuffer[c][a] = timesample[c] * header->lsbInMv;
            unFilteredBuffer[c][a] = decompressBuffer[c][a];
            if(c <= MAX_NUM_CHANNELS) {
                ecgFilter[c].putSample(decompressBuffer[c][a]);
                if(ecgFilter[c].isOutputAvailable()) {
                    filteredSampleNum[c]++;
                    decompressBuffer[c][a] = ecgFilter[c].getSample();
                }
            }
        }
    }

    for(int a = 0; a < filteredSampleNum[0]; a++) {
        #ifdef DEBUGFILE
            EcgProcessor::writeDebugDataToFile(&unFilteredBuffer[0][a], &decompressBuffer[0][a]);
        #endif

        EcgProcessor::calculate12Channels(&decompressBuffer[0][a], &decompressBuffer[0][a], DECOMPRESS_BUFFER_STRIDE);
    }

    EcgArea::instance().putData((GLfloat*)decompressBuffer, header->channelCount, filteredSampleNum[0], DECOMPRESS_BUFFER_STRIDE);
    pulseCurrentBPM = (int)pulse_current_bpm;
    LOGD("BPM: %d ", pulseCurrentBPM);
}

float EcgProcessor::getSamplingFrequency(){
    return samplingFrequency;
}

void EcgProcessor::calculate12Channels(float *input, float *output, int stride) {
    float I = input[1*stride];
    float II = input[2*stride];
    float III = II - I;
    float V1 = input[7*stride];
    float V2 = input[3*stride];
    float V3 = input[4*stride];
    float V4 = input[5*stride];
    float V5 = input[6*stride];
    float V6 = input[0*stride];
    float aVR = (-II-I)/3;
    float aVL = (I-III)/3;
    float aVF = (II+III)/3;

    EcgProcessor::calculateBPM(II);

    output[0*stride] = I;
    output[1*stride] = II;
    output[2*stride] = III;
    output[3*stride] = aVR;
    output[4*stride] = aVL;
    output[5*stride] = aVF;
    output[6*stride] = V1;
    output[7*stride] = V2;
    output[8*stride] = V3;
    output[9*stride] = V4;
    output[10*stride] = V5;
    output[11*stride] = V6;
}

void EcgProcessor::writeDebugDataToFile(float *inputBefore, float *inputAfter) {
    #ifdef DEBUGFILE
    int stride = DECOMPRESS_BUFFER_STRIDE;
    float I = inputBefore[1*stride];
    float II = inputBefore[2*stride];
    float V1 = inputBefore[7*stride];
    float V2 = inputBefore[3*stride];
    float V3 = inputBefore[4*stride];
    float V4 = inputBefore[5*stride];
    float V5 = inputBefore[6*stride];
    float V6 = inputBefore[0*stride];

    float Ia = inputAfter[1*stride];
    float IIa = inputAfter[2*stride];
    float V1a = inputAfter[7*stride];
    float V2a = inputAfter[3*stride];
    float V3a = inputAfter[4*stride];
    float V4a = inputAfter[5*stride];
    float V5a = inputAfter[6*stride];
    float V6a = inputAfter[0*stride];

    fprintf(bf, "%s,%f,%f,%f,%f,%f,%f,%f,%f\n", getCurrentTime(), I, II, V1, V2, V3, V4, V5, V6);
    fprintf(af, "%s,%f,%f,%f,%f,%f,%f,%f,%f\n", getCurrentTime(), Ia, IIa, V1a, V2a, V3a, V4a, V5a, V6a);
    #endif
}

int EcgProcessor::calculateBPM(float value) {
    if (value >= PULSE_RESET_THRESHOLD) {
        pulse_state = PULSE_IDLE;
        pulse_current_timestamp = 0;
        pulse_last_timestamp = 0;
        pulse_previous_value = 0;
        pulse_current_bpm = 0.0;
        pulse_beats = 0;
        pulse_present = 0;
        return 0;
    }

    // If no pulse detected for some time, reset.
    if (pulse_beats > 0 && getClock() - pulse_last_timestamp > PULSE_RESET_TIMEOUT) {
        pulse_current_bpm = 0.0;
        pulse_beats = 0;
        pulse_present = 0;
    }

    switch (pulse_state) {
        case PULSE_IDLE: {
            // Idle state: we wait for the value to cross the threshold.
            if (value >= PULSE_THRESHOLD) {
                pulse_state = PULSE_RISING;
            }
            break;
        }
        case PULSE_FALLING: {
            // Move into idle state when under the threshold.
            if (value < PULSE_THRESHOLD) {
                pulse_state = PULSE_IDLE;
            }
            break;
        }
        case PULSE_RISING: {
            if (value > pulse_previous_value) {
                // Still rising.
                pulse_current_timestamp = getClock();
            }
            else {
                // Reached the top.
                uint32_t beat_duration = pulse_current_timestamp - pulse_last_timestamp;
                pulse_last_timestamp = pulse_current_timestamp;

                // Compute BPM.
                float raw_bpm = 60000.0 / (float) beat_duration;
                if (raw_bpm > 10.0 && raw_bpm < 300.0) {
                    pulse_beats++;
                    //float bpm = filter_mean(&rolling_mean_pulse, raw_bpm, 0);
                    float bpm = raw_bpm;
                    if (pulse_beats > PULSE_INITIAL_BEATS) {
                        pulse_current_bpm = bpm;
                        pulse_beats = PULSE_INITIAL_BEATS;
                        pulse_present = 1;
                    }
                }

                pulse_state = PULSE_FALLING;
                return 1;
            }
            break;
        }
    }

    pulse_previous_value = value;
    return 0;
}