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
 
#include "EcgArea.h"
#include "log.h"
#include "EcgProcessor.h"

int mains_frequency = 0;

EcgArea::EcgArea():
        pixelDensity(100,100){

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
    /*
    drawableList.push_back(&pause_button);
    pause_button.setZOrder(2);
    pause_button.setSize(BUTTON_SIZE);
    pause_button.setColor(0.1, 0.2, 0.6);
    */
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

    /*
    for (int a=0; a<ECG_CURVE_COUNT; a++) {
        drawableList.push_back(&outOfRangeLabels[a]);
        outOfRangeLabels[a]
                .setColor(Image::BLACK)
                .setTextSizeMM(3.5);
    }
    */
    /*
    drawableList.push_back(&pause_label);
    pause_label
            .setColor(Image::BLACK)
            .setTextSizeMM(5.5);
    */
    drawableList.push_back(&devLabel);
    devLabel
        .setColor(Image::GREY)
        .setTextSizeMM(3.5);

    drawableList.push_back(&speed_warning_label);
    speed_warning_label
        .setColor(Image::GREY)
        .setTextSizeMM(3.5);

    padInCm=0.5;
    ecgCmPerMv = 2.0;
    ecgCmPerSec = 2.5;
    lastSampleFrequency=0;
    cur_column = 0;

    drawableList.push_back(&disconnectedLabel);
    disconnectedLabel
            .setColor(Image::GREY)
            .setTextSizeMM(4.5);

    deviceDisconnected();
    selected_layout = NORMAL_LAYOUT;
}

EcgArea &EcgArea::instance(){
    static EcgArea area;
    return area;
}

void EcgArea::init(AAssetManager *assetManager, int mains_freq){
    LOGD("HEH: EcgArea::init");
    redraw();
    DrawableGroup::init(assetManager);
    mains_frequency = mains_freq;
}

void EcgArea::rescale(){
    LOGD("HEH: EcgArea::rescale");
    lastSampleFrequency=EcgProcessor::instance().getSamplingFrequency();

    float xScale = ecgCmPerSec * pixelDensity.x / lastSampleFrequency;
    float yScale = ecgCmPerMv * pixelDensity.y;

    for (int a=0; a<ECG_CURVE_COUNT; a++) {
        ecgCurves[a].setScale(xScale, yScale);
        labels[a].drawText(labelText[a]);
        //outOfRangeLabels[a].drawText("OUT OF RANGE");
    }
    rhythm.setScale(xScale, yScale);
    rhythm_label.drawText(rhythm_text);
    //availableHeight = labels[1].getYPosition() - labels[0].getYPosition();

    //pause_label.drawText(pause_text);

    disconnectedLabel.drawText("DISCONNECTED");
    devLabel.drawText("DEV VERSION " GIT_HASH " - " __DATE__ );
    speed_warning_label.drawText("PAPER SPEED: default");
}


void EcgArea::constructLayout() {   // TODO
    LOGD("HEH: EcgArea::constructLayout");
    resetContent();
    if (selected_layout == NORMAL_LAYOUT) {
        constructLayoutNormal();
    }
    else if (selected_layout == RHYTHM_LAYOUT) {
        constructLayoutRhythm();
    }
}


void EcgArea::constructLayoutNormal(){
    LOGD("HEH: EcgArea::constructLayoutNormal");
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

    disconnectedLabel.setPosition((screenSize.w - disconnectedLabel.getWidth())/2, screenSize.h/2 - disconnectedLabel.getHeight());

    int padInPixels=padInCm*pixelDensity.x;
    int curveWidth=(activeArea.width()-(c-1)*padInPixels)/c;

    const int yStep = activeArea.height()/r;
    const int xStep = curveWidth+padInPixels;

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
        labels[a].setPosition(xCoord + 10, yCoord - 0.8*pixelDensity.y);
        //outOfRangeLabels[a].setPosition(xCoord + 20, yCoord - 0.4*pixelDensity.y);
        //curvePositions[a] = yCoord;
        //timers[a] = 0;
        if (a == 1 || a == 3 || a > 4) {
            ecgCurves[a].setVisible(true);
            labels[a].setVisible(true);
        }
        else if (deviceNotConnected) {
            ecgCurves[a].setVisible(true);
            labels[a].setVisible(true);
            rhythm.setVisible(true);
            rhythm_label.setVisible(true);
        }
    }

    rhythm.setLength(curveWidth*ECG_COLUMN_COUNT);
    const int rhy_x = activeArea.left();
    const int rhy_y = activeArea.top() + 3*yStep + yStep/2;
    rhythm.setPosition(rhy_x, rhy_y);
    rhythm_label.setPosition(rhy_x + 10, rhy_y - 0.8*pixelDensity.y);
    rhy_remains = OK_REMAINS;   // OPTION 2
    /*
    for (int b = 0; b < BUTTONS_COUNT; b++) {

    }
     */
    //const int button_offset = 85;
    //const int button_size = BUTTON_SIZE * 8;
    //pause_button.setPosition(button_offset, screenSize.h - button_offset);
    //pause_label.setPosition(85 + 160, screenSize.h - 90);

    //availableHeight = labels[1].getYPosition() - labels[0].getYPosition();
    //LOGI("Available height: %d\n", availableHeight);

    devLabel.setPosition(10, 0);
    speed_warning_label.setPosition(screenSize.w - speed_warning_label.getWidth() - 10, 0);

    //pause_size = pause_button.getSize();
}

void EcgArea::constructLayoutRhythm(){
    LOGD("HEH: EcgArea::constructLayoutRhythm");
    int r,c;
    // only displaying 4 signals (I, III, aVL and rhythm)
    c = 1;
    r = 4;

    disconnectedLabel.setPosition((screenSize.w - disconnectedLabel.getWidth())/2, screenSize.h/2 - disconnectedLabel.getHeight());

    int padInPixels=padInCm*pixelDensity.x;
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
            labels[a].setPosition(xCoord + 10, yCoord - 0.8*pixelDensity.y);
        }
    }

    rhythm.setLength(curveWidth);
    const int rhy_x = activeArea.left();
    const int rhy_y = activeArea.top() + 3*yStep + yStep/2;
    rhythm.setPosition(rhy_x, rhy_y);
    rhythm_label.setPosition(rhy_x + 10, rhy_y - 0.8*pixelDensity.y);
    rhy_remains = OK_REMAINS;   // OPTION 2
    /*
    for (int b = 0; b < BUTTONS_COUNT; b++) {

    }
     */
    //const int button_offset = 85;
    //const int button_size = BUTTON_SIZE * 8;
    //pause_button.setPosition(button_offset, screenSize.h - button_offset);
    //pause_label.setPosition(85 + 160, screenSize.h - 90);

    //availableHeight = labels[1].getYPosition() - labels[0].getYPosition();
    //LOGI("Available height: %d\n", availableHeight);

    devLabel.setPosition(10, 0);
    speed_warning_label.setPosition(screenSize.w - speed_warning_label.getWidth() - 10, 0);
}

void EcgArea::contextResized(int w, int h){
    LOGD("HEH: EcgArea::contextResized");
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
    LOGD("HEH: EcgArea::calculateUnalignedArea");
    int unalignedPixels=(((float)size) / dpcm);
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

void EcgArea::putData(GLfloat *data, int nChannels, int nPoints, int stride){
    //LOGD("HEH: EcgArea::putData");

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
        rhythm.put(data + stride*1, nPoints);
        rhythm_circle.setPosition(rhythm.endpointCoordinates());
    }
    else {
        int remains = OK_REMAINS;
        for (int a=0; a<3; a++) {
            //LOGI("TEST: curve nr %d", cur_column*3 + a);
            remains = ecgCurves[cur_column*3 + a].put(data + stride*a, nPoints);
            //LOGI("TEST: remains: %d", remains);
            endpointCircles[cur_column*3 + a].setPosition(ecgCurves[cur_column*3 + a].endpointCoordinates());
        }
        //LOGI("TEST: Num of points: %d, remains: %d, col: %d\n", nPoints, remains, cur_column);

        // OPTION 2 - to update rhythm
        if (rhy_remains == OK_REMAINS || rhy_remains == -1) {
            //LOGI("TEST: rhythm draw");
            rhy_remains = rhythm.put(data + stride*1, nPoints);
            rhythm_circle.setPosition(rhythm.endpointCoordinates());
        }

        int rhythm_points = nPoints;
        if (remains == NO_REMAINS) {    // switching column of signals
            // OPTION 1 - leaving out parts of signal when changing columns
            //int curve_cur_position = ecgCurves[cur_column*3].getEndCoordinateX();  // get current (before switch) signal x coord from 1. line
            //int rhythm_cur_position = rhythm.getEndCoordinateX() - curve_cur_position*cur_column;   // get current rhythm signal position and substitute old curves max positions
            //rhythm_points = curve_cur_position - rhythm_cur_position;   // calculate number of points to put into rhythm curve

            //LOGI("TEST: curve_cur_position: %d, rhythm_cur_position: %d, rhythm_points: %d\n", curve_cur_position, rhythm_cur_position, rhythm_points);
            cur_column++;
            if (cur_column == ECG_COLUMN_COUNT) {
                //LOGI("TEST: back to start...");
                cur_column = 0;
                rhy_remains = OK_REMAINS;
                // OPTION 2 - reset rhythm drawing to 0
                rhythm.resetCurrWritePos();
                rhythm_circle.setPosition(Vec2<int>());

                // OPTION 1
                //rhythm_points++; // we add 1 point when switching back to 1. column to match requiredNumOfPoints for other curves
            }
            /*
            for (int a=0; a<3; a++) {
                ecgCurves[cur_column*3 + a].resetCurrWritePos();
            }*/
        }
        // OPTION 1 - to update rhythm
        //rhythm.put(data + stride*1, rhythm_points);
        //rhythm_circle.setPosition(rhythm.endpointCoordinates());
    }
}

void EcgArea::draw(){
    //LOGD("HEH: EcgArea::draw");
    if (lastSampleFrequency!=EcgProcessor::instance().getSamplingFrequency()){
        rescale();
    }
    /* // Out of range - not used anymore
    for (int i=0; i<12; i++) {
        int circlePosition = endpointCircles[i].getYPosition();
        int offset = 100;    // how much can the signal be outside available space
        int min = curvePositions[i] - (availableHeight / 2) - offset;
        int max = curvePositions[i] + (availableHeight / 2) + offset;
        timers[i]++;

        //LOGI("Drawing Curves - Circle: %d, Curve: %d, Min: %d, Max %d\n", circlePosition, curvePositions[i], min, max);

        if (circlePosition < min || circlePosition > max) {
            timers[i] = 0;
            if (ecgCurves[i].getVisible()) {
                endpointCircles[i].setVisible(false);
                ecgCurves[i].setVisible(false);
                outOfRangeLabels[i].setVisible(true);
            }
        }
        else {
            if (!ecgCurves[i].getVisible() && timers[i] > 50) {
                endpointCircles[i].setVisible(true);
                ecgCurves[i].setVisible(true);
                outOfRangeLabels[i].setVisible(false);
            }
        }
    }
    */
    DrawableGroup::draw();
    redrawNeeded=false;
}

void EcgArea::redraw(){
    LOGD("HEH: EcgArea::redraw");
    redrawNeeded=true;
}

bool EcgArea::isRedrawNeeded(){
    return redrawNeeded;
}

void EcgArea::setContentVisible(bool visible){
    LOGD("HEH: EcgArea::setContentVisible");
    if (selected_layout == NORMAL_LAYOUT) {
        for (int a=0; a<ECG_CURVE_COUNT; a++) {
            endpointCircles[a].setVisible(visible);
            ecgCurves[a].setVisible(visible);
            labels[a].setVisible(visible);
            //outOfRangeLabels[a].setVisible(false);
        }
        rhythm_circle.setVisible(visible);
        rhythm.setVisible(visible);
        rhythm_label.setVisible(visible);
    }
    else {/*
        endpointCircles[0].setVisible(visible);
        ecgCurves[0].setVisible(visible);
        labels[0].setVisible(visible);
        endpointCircles[2].setVisible(visible);
        ecgCurves[2].setVisible(visible);
        labels[2].setVisible(visible);
        endpointCircles[4].setVisible(visible);
        ecgCurves[4].setVisible(visible);
        labels[4].setVisible(visible);
        rhythm_circle.setVisible(visible);
        rhythm.setVisible(visible);
        rhythm_label.setVisible(visible); */
    }
    //pause_button.setVisible(visible);
    //pause_label.setVisible(visible);
}

void EcgArea::deviceConnected(){
    LOGD("HEH: EcgArea::deviceConnected");
    setContentVisible(true);
    disconnectedLabel.setVisible(false);
    deviceNotConnected = false;
}

void EcgArea::deviceDisconnected(){
    LOGD("HEH: EcgArea::deviceDisconnected");
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
    LOGD("HEH: EcgArea::changeLayout");
    resetContent();
    if (selected_layout == NORMAL_LAYOUT) {
        selected_layout = RHYTHM_LAYOUT;
    }
    else if (selected_layout == RHYTHM_LAYOUT) {
        selected_layout = NORMAL_LAYOUT;
    }
    contextResized(screenSize.w, screenSize.h);
}

/*
int * EcgArea::getButtonsSize() {
    //return pause_size;
}
 */