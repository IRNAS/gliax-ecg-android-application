/*
 * This file is part of gliax-ecg-android-application
 * Glia is a project with the goal of releasing high quality free/open medical hardware
 * to increase availability to those who need it.
 * For more information visit Glia Free Medical hardware webpage: https://glia.org/
 *
 * Made by Institute Irnas (https://www.irnas.eu/)
 * Copyright (C) 2019 Vid Rajtmajer
 *
 * Based on MobilECG, an open source clinical grade Holter ECG.
 * For more information visit http://mobilecg.hu
 * Authors: Robert Csordas, Peter Isza
 *
 * This project uses modified version of usb-serial-for-android driver library
 * to communicate with Irnas made ECG board.
 * Original source code: https://github.com/mik3y/usb-serial-for-android
 * Library made by mik3y and kai-morich, modified by Vid Rajtmajer
 * Licensed under LGPL Version 2.1
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
 
#include "EcgArea.h"
#include "log.h"
#include "EcgProcessor.h"

#include <stdio.h>

EcgArea::EcgArea():
        pixelDensity(100,100){

    ecgCmPerMv = 2.0;
    ecgCmPerSec = 2.5;
    lastSampleFrequency=0;
    cur_column = 0;
    bpm_num_width = 0;

    grid.setZOrder(10);
    drawableList.push_back(&grid);

    //Use separator for loops to avoid mixing the order of
    //curves, circles and texts to eliminate unnecessary shader switches.
    for (int a=0; a<ECG_CURVE_COUNT; a++) {
        drawableList.push_back(&ecgCurves[a]);
        ecgCurves[a].setZOrder(1);
    }

    drawableList.push_back(&rhythm);
    rhythm.setZOrder(1);

    for (int a=0; a<ECG_CURVE_COUNT; a++) {
        drawableList.push_back(&endpointCircles[a]);
        endpointCircles[a].setZOrder(0);
    }

    drawableList.push_back(&rhythm_circle);
    rhythm_circle.setZOrder(0);

    for (int a=0; a<ECG_CURVE_COUNT; a++) {
        drawableList.push_back(&labels[a]);
        labels[a]
                .setColor(Image::BLACK)
                .setTextSizeMM(3.5);
    }

    drawableList.push_back(&rhythm_label);
    rhythm_label
            .setColor(Image::BLACK)
            .setTextSizeMM(3.5);

    drawableList.push_back(&devLabel);
    devLabel
        .setColor(Image::GREY)
        .setTextSizeMM(3.5);

    drawableList.push_back(&speed_warning_label);
    speed_warning_label
        .setColor(Image::GREY)
        .setTextSizeMM(4.5);

    drawableList.push_back(&disconnectedLabel);
    disconnectedLabel
            .setColor(Image::GREY)
            .setTextSizeMM(4.5);

    drawableList.push_back(&bpm_label);
    bpm_label
        .setColor(Image::TRANSPARENT)
        .setTextSizeMM(5.5);

    drawableList.push_back(&hr_label);
    hr_label
        .setColor(Image::TRANSPARENT)
        .setTextSizeMM(9.5);

    drawableList.push_back(&bpm_num);
    bpm_num
        .setColor(Image::TRANSPARENT)
        .setTextSizeMM(11.5);

    deviceDisconnected();
    selected_layout = NORMAL_LAYOUT;

    rhy_screen_full = 0;
}

EcgArea &EcgArea::instance(){
    static EcgArea area;
    return area;
}

void EcgArea::init(AAssetManager *assetManager){
    //LOGD("HEH: EcgArea::init");
    redraw();
    DrawableGroup::init(assetManager);
}

void EcgArea::rescale(){
    //LOGD("HEH: EcgArea::rescale");
    lastSampleFrequency=EcgProcessor::instance().getSamplingFrequency();

    float xScale = ecgCmPerSec * pixelDensity.x / lastSampleFrequency;
    float yScale = ecgCmPerMv * pixelDensity.y;

    for (int a=0; a<ECG_CURVE_COUNT; a++) {
        ecgCurves[a].setScale(xScale, yScale);
        labels[a].drawText(labelText[a]);
    }
    rhythm.setScale(xScale, yScale);
    rhythm_label.drawText(rhythm_text);
    //availableHeight = labels[1].getYPosition() - labels[0].getYPosition();

    disconnectedLabel.drawText("DISCONNECTED");
    devLabel.drawText("v. " GIT_HASH " - " __DATE__ );
    speed_warning_label.drawText("25 mm/s");

    hr_label.drawText("HR ");
    bpm_num.drawText("000");
    bpm_label.drawText(" bpm");
    bpm_num_width = bpm_num.getWidth();
}

void EcgArea::constructLayout() {
    //LOGD("HEH: EcgArea::constructLayout");
    resetContent();
    rhy_screen_full = 0;

    hr_label.setPosition((screenSize.w - hr_label.getWidth() - bpm_num_width - bpm_label.getWidth())/2, screenSize.h/2 - (hr_label.getHeight()/2) - 12);
    bpm_num.setPosition((screenSize.w + hr_label.getWidth() - bpm_num_width - bpm_label.getWidth())/2, screenSize.h/2 - (bpm_num.getHeight()/2) - 20);
    bpm_label.setPosition((screenSize.w +  hr_label.getWidth() + bpm_num_width - bpm_label.getWidth() + 3)/2, screenSize.h/2 - (bpm_label.getHeight()/2));

    disconnectedLabel.setPosition((screenSize.w - disconnectedLabel.getWidth())/2, screenSize.h/2 - disconnectedLabel.getHeight());
    devLabel.setPosition(10, 0);
    speed_warning_label.setPosition(screenSize.w - speed_warning_label.getWidth() - 10, 0);

    if (!deviceNotConnected) {
        if (selected_layout == NORMAL_LAYOUT) {
            constructLayoutNormal();
        }
        else if (selected_layout == RHYTHM_LAYOUT) {
            constructLayoutRhythm();
        }
    }
}

void EcgArea::constructLayoutNormal(){
    //LOGD("HEH: EcgArea::constructLayoutNormal");
    cur_column = 0;
    int r,c;
    /*  // app orientation is locked to landscape
    if (activeArea.width()<activeArea.height()){
        c=2;
    } else {
        c=4;
    }
    */
    c = ECG_COLUMN_COUNT;  // num of columns
    r = (ECG_CURVE_COUNT+c-1)/c + 1;  // num of rows (3 for all 12 signals + 1 for rhythm)

    int padInPixels = 0;
    int curveWidth=(activeArea.width()-(c-1)*padInPixels)/c;

    const int yStep = activeArea.height()/r;
    //const int xStep = curveWidth+padInPixels;
    const int xStep = curveWidth;

    const int bottomCurves = r % 3;
    const int topCurves = r - bottomCurves;

    const int topCurveCount = topCurves * c;

    for (int a=0; a<ECG_CURVE_COUNT; a++){
        ecgCurves[a].setLength(curveWidth);

        const bool bottom = a >= topCurveCount;

        const int x=bottom ? (c - 1 - ((a - topCurveCount) / bottomCurves)) : (a / topCurves);
        const int y=bottom ? ((a - topCurveCount) % bottomCurves + topCurves) : (a % topCurves);

        const int xCoord=activeArea.left() + x*xStep;
        const int yCoord=activeArea.top() + y*yStep + yStep/2;

        ecgCurves[a].setPosition(xCoord, yCoord);
        labels[a].setPosition(xCoord + 10, (float)(yCoord - 0.8*pixelDensity.y));

        //if (!ecgCurves[a].getVisible()) {
        ecgCurves[a].setVisible(true);
        labels[a].setVisible(true);
        //}
    }

    rhythm.setLength(curveWidth*ECG_COLUMN_COUNT);
    const int rhy_x = activeArea.left();
    const int rhy_y = activeArea.top() + 3*yStep + yStep/2;
    rhythm.setPosition(rhy_x, rhy_y);
    rhythm_label.setPosition(rhy_x + 10, (float)(rhy_y - 0.8*pixelDensity.y));
    rhy_remains = OK_REMAINS;

    if (!rhythm.getVisible()) {
        rhythm.setVisible(true);
        rhythm_label.setVisible(true);
    }
}

void EcgArea::constructLayoutRhythm(){
    //LOGD("HEH: EcgArea::constructLayoutRhythm");
    int r;
    // only displaying 4 signals (I, III, aVL and rhythm)
    r = 4;

    int curveWidth=(activeArea.width());
    const int yStep = activeArea.height()/r;
    int curRow = 0;

    for (int a=0; a<ECG_CURVE_COUNT; a++){
        if (a == 1 || a == 3 || a > 4) {
            ecgCurves[a].setVisible(false);
            labels[a].setVisible(false);
        }
        else {
            ecgCurves[a].setLength(curveWidth);
            const int xCoord=activeArea.left();
            const int yCoord=activeArea.top() + curRow*yStep + yStep/2;
            curRow++;
            ecgCurves[a].setPosition(xCoord, yCoord);
            labels[a].setPosition(xCoord + 10, (float)(yCoord - 0.8*pixelDensity.y));
        }
    }

    rhythm.setLength(curveWidth);
    const int rhy_x = activeArea.left();
    const int rhy_y = activeArea.top() + 3*yStep + yStep/2;
    rhythm.setPosition(rhy_x, rhy_y);
    rhythm_label.setPosition(rhy_x + 10, (float)(rhy_y - 0.8*pixelDensity.y));
    rhy_remains = OK_REMAINS;

    if (!rhythm.getVisible()) {
        rhythm.setVisible(true);
        rhythm_label.setVisible(true);
    }
}

void EcgArea::contextResized(int w, int h){
    //LOGD("HEH: EcgArea::contextResized");
    int deleteX=calculateUnalignedArea(w, pixelDensity.x);
    int deleteY=calculateUnalignedArea(h, pixelDensity.y);

    screenSize.w=w;
    screenSize.h=h;

    activeArea=Rect(deleteX/2, deleteY/2, w-deleteX+1, h+1-deleteY);

    redraw();
    DrawableGroup::contextResized(w,h);
    constructLayout();
}


int EcgArea::calculateUnalignedArea(int size, float dpcm){
    //LOGD("HEH: EcgArea::calculateUnalignedArea");
    int unalignedPixels=(int)(size / dpcm);
    return size-(int)(unalignedPixels*dpcm);
}

const Rect &EcgArea::getActiveArea(){
    return activeArea;
}

const Vec2<float> &EcgArea::getPixelDensity(){
    return pixelDensity;
}

void EcgArea::setPixelDensity(const Vec2<float> &pPixelDensity){
    pixelDensity=pPixelDensity;
    rescale();
}

void EcgArea::putData(GLfloat *data, int nChannels, int nPoints, int stride, int bpm, int cur_time){
    //LOGD("HEH: EcgArea::putData");
    if (ecgCmPerSec == 1.25 && cur_time - last_color_change > 1000) {
        last_color_change = cur_time;
        if (x_speed_color) {
            speed_warning_label.setColor(Image::GREY);
            x_speed_color = 0;
        }
        else {
            speed_warning_label.setColor(Image::RED);
            x_speed_color = 1;
        }
    }

    char speed_text[6];
    if (ecgCmPerSec == 1.25) {
        sprintf(speed_text, "%d mm/s", 50);
    }
    else {
        speed_warning_label.setColor(Image::GREY);
        sprintf(speed_text, "%d mm/s", (int)(ecgCmPerSec*10));
    }
    speed_warning_label.drawText(speed_text);

    // display heart rate if in range
    char bpm_text[3];
    if (bpm >= 30 && bpm <= 250) {
        sprintf(bpm_text, "%d", bpm);
    }
    else {
        sprintf(bpm_text, "%s", " --- ");
    }
    bpm_num.drawText(bpm_text);

    // display selected layout
    if (selected_layout == RHYTHM_LAYOUT) {
        for (int a=0; a<ECG_CURVE_COUNT; a++) {
            if (a == 1 || a == 3 || a > 4) {
                //ecgCurves[a].resetCurrWritePos();
                //endpointCircles[a].setPosition(Vec2<int>());
            }
            else {
                ecgCurves[a].put(data + stride*a, nPoints);
                endpointCircles[a].setPosition(ecgCurves[a].endpointCoordinates());
            }
        }
        layout_remains = rhythm.put(data + stride*1, nPoints);
        rhythm_circle.setPosition(rhythm.endpointCoordinates());

        //LOGI("HEH: layout remains: %d", layout_remains);
        if (layout_remains != OK_REMAINS) {
            rhy_screen_full = 1;
        }
    }
    else {
        int remains = OK_REMAINS;
        for (int a=0; a<3; a++) {
            remains = ecgCurves[cur_column*3 + a].put(data + stride*(cur_column*3 + a), nPoints);

            // following two lines are used for testing
            //remains = ecgCurves[cur_column*3 + a].put(data + stride*1, nPoints);
            //LOGI("TEST: remains: %d", remains);

            endpointCircles[cur_column*3 + a].setPosition(ecgCurves[cur_column*3 + a].endpointCoordinates());
        }
        rhy_remains = rhythm.put(data + stride*1, nPoints);
        rhythm_circle.setPosition(rhythm.endpointCoordinates());
        //LOGI("TEST: Num of points: %d, remains: %d, col: %d\n", nPoints, remains, cur_column);

        if (remains != OK_REMAINS && remains != -1) {    // switching column of signals
            //LOGI("TEST: curve_cur_position: %d, rhythm_cur_position: %d, rhythm_points: %d\n", curve_cur_position, rhythm_cur_position, rhythm_points);
            cur_column++;
            if (cur_column == ECG_COLUMN_COUNT) {
                cur_column = 0;
                rhy_remains = OK_REMAINS;
            }
            else {
                for (int a=0; a<3; a++) {   // put all current points remaining from previous column into next one
                    ecgCurves[cur_column*3 + a].put(data + stride*(cur_column*3 + a) + nPoints - remains, remains);

                    // following two lines are used for testing
                    //int remains_2 = ecgCurves[cur_column*3 + a].put(data + stride*1 + nPoints - remains, remains);
                    //LOGI("TEST: change col, remains 2: %d", remains_2);

                    endpointCircles[cur_column*3 + a].setPosition(ecgCurves[cur_column*3 + a].endpointCoordinates());
                }
            }
        }
    }
}

void EcgArea::draw(){
    //LOGD("HEH: EcgArea::draw");
    if (lastSampleFrequency!=EcgProcessor::instance().getSamplingFrequency()){
        rescale();
    }
    DrawableGroup::draw();
    redrawNeeded=false;
}

void EcgArea::redraw(){
    //LOGD("HEH: EcgArea::redraw");
    redrawNeeded=true;
}

bool EcgArea::isRedrawNeeded(){
    return redrawNeeded;
}

void EcgArea::setContentVisible(bool visible){
    LOGD("TEST: EcgArea::setContentVisible");
    hr_label.setVisible(visible);
    bpm_label.setVisible(visible);
    bpm_num.setVisible(visible);

    for (int a=0; a<ECG_CURVE_COUNT; a++) {
        endpointCircles[a].setVisible(visible);
        ecgCurves[a].setVisible(visible);
        labels[a].setVisible(visible);
    }
    rhythm_circle.setVisible(visible);
    rhythm.setVisible(visible);
    rhythm_label.setVisible(visible);
}

void EcgArea::deviceConnected(){
    //LOGD("HEH: EcgArea::deviceConnected");
    setContentVisible(true);
    disconnectedLabel.setVisible(false);
    deviceNotConnected = false;
}

void EcgArea::deviceDisconnected(){
    //LOGD("HEH: EcgArea::deviceDisconnected");
    setContentVisible(false);
    disconnectedLabel.setVisible(true);
    deviceNotConnected = true;
}

void EcgArea::resetContent() {
    cur_column = 0;
    for (int i = 0; i < ECG_CURVE_COUNT; i++) {
        endpointCircles[i].setPosition(Vec2<int>());
        ecgCurves[i].resetCurrWritePos();
    }
    rhythm_circle.setPosition(Vec2<int>());
    rhythm.resetCurrWritePos();
}

void EcgArea::changeLayout() {
    //LOGD("HEH: EcgArea::changeLayout");
    resetContent();
    if (selected_layout == NORMAL_LAYOUT) {
        selected_layout = RHYTHM_LAYOUT;
    }
    else if (selected_layout == RHYTHM_LAYOUT) {
        selected_layout = NORMAL_LAYOUT;
    }
    setContentVisible(true);
    contextResized(screenSize.w, screenSize.h);
}

int EcgArea::getCurrentLayout() {
    return selected_layout;
}

void EcgArea::setSpeed(float speed) {
    ecgCmPerSec = speed;
}

int EcgArea::getRhyScreenFull() {
    int state = rhy_screen_full;
    rhy_screen_full = 0;
    return state;
}
