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

#include "Curve.h"
#include "EcgArea.h"
#include "Helper.h"
#include "ShaderBuilder.h"
#include <algorithm>
#include "log.h"

int Curve::xCoordinatesLength=0;
GLfloat *Curve::xCoordinates=NULL;
GLuint Curve::xCoordinatesOnGPU=0;
const float Curve::POINT_INVALID=0.0/0.0;

std::vector<GLfloat> Curve::invalidBuffer(1000, Curve::POINT_INVALID);

GLuint Curve::getXCoordinates(int capacity){
    capacity = std::max(capacity, 10000);
    if (capacity>xCoordinatesLength || EcgArea::instance().isRedrawNeeded()){
        if (xCoordinatesLength==0 || EcgArea::instance().isRedrawNeeded()){
            glGenBuffers(1, &xCoordinatesOnGPU);
        }

        if (xCoordinatesLength!=0){
            delete [] xCoordinates;
        }

        xCoordinates = new GLfloat[capacity];
        xCoordinatesLength = capacity;

        for (int a=0; a<capacity; a++){
            xCoordinates[a]=a;
        }

        glBindBuffer(GL_ARRAY_BUFFER , xCoordinatesOnGPU);
        glBufferData(GL_ARRAY_BUFFER , sizeof(GLfloat)*capacity, xCoordinates, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER , 0);
    }

    return xCoordinatesOnGPU;
}

Curve::Curve(): DrawableObject(){
    scale.x=1;
    scale.y=3;
    position.y=100;
    currNumOfPoints=0;
    requiredNumOfPoints=1;
    clearWidthInPoints=10;  // was set to 100
    endCoordinates.x=0;
    endCoordinates.y=0;

    color[0]=1;
    color[1]=0;
    color[2]=0;

    reset = 0;
}

void Curve::init(AAssetManager *assetManager){
    vertexShader = helper::loadAsset(assetManager, "curve.vert");
    fragmentShader = helper::loadAsset(assetManager, "curve.frag");
}

void Curve::clear(){
    //Fill value with invalid data
    glBindBuffer(GL_ARRAY_BUFFER , valueBuffer);
    for (int offset=0; offset<currNumOfPoints; offset+=invalidBuffer.size()){
        int transferblock = std::min((int)invalidBuffer.size(), currNumOfPoints - offset);
        glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(GLfloat), transferblock*sizeof(GLfloat), invalidBuffer.data());
    }
    glBindBuffer(GL_ARRAY_BUFFER , 0);
}

void Curve::setLength(int pLengthInPixels){
    lengthInPixels = pLengthInPixels;
    requiredNumOfPoints = std::max((int)(lengthInPixels / scale.x),1);
}

void Curve::glInit(){
    shaderId = ShaderBuilder::instance().buildShader("Curve", vertexShader, fragmentShader);
    GLuint shaderProgram = ShaderBuilder::instance().getShader(shaderId);

    shader_a_Position=helper::getGlAttributeWithAssert(shaderProgram, "a_Position");
    shader_screenSize=helper::getGlUniformWithAssert(shaderProgram, "screenSize");
    shader_position=helper::getGlUniformWithAssert(shaderProgram, "position");
    shader_scale=helper::getGlUniformWithAssert(shaderProgram, "scale");
    shader_color=helper::getGlUniformWithAssert(shaderProgram, "color");
    shader_a_Value=helper::getGlAttributeWithAssert(shaderProgram, "a_Value");

    shader_endOffset=helper::getGlUniformWithAssert(shaderProgram, "endOffset");
    shader_clearWidth=helper::getGlUniformWithAssert(shaderProgram, "clearWidth");
    shader_pointCount=helper::getGlUniformWithAssert(shaderProgram, "pointCount");

    glGenBuffers(1, &valueBuffer);

    getXCoordinates(currNumOfPoints);
}

int Curve::put(GLfloat *data, int n){
    if (n<=0)
        return -1;

    if (reset) {
        currWritePos = 0;
        reset = 0;
    }

    int new_points_count = n;
    int return_value = OK_REMAINS;
    if ((currWritePos + n) >= requiredNumOfPoints) {
        new_points_count = requiredNumOfPoints - currWritePos;
        //LOGI("TEST: num of points that fit: %d", new_points_count);
        return_value = n - new_points_count;
        //LOGI("TEST: num of points that don't fit: %d", return_value);
    }
    newPointBuffer.add(data, new_points_count);
    int new_position = currWritePos+newPointBuffer.used();

    endCoordinates.x = new_position;
    endCoordinates.y = data[new_points_count-1];

    //LOGI("TEST: requiredNumOfPoints: %d, currWritePos: %d, newPos: %d\n", requiredNumOfPoints, currWritePos, new_position);
    return return_value;
}

void Curve::resizeOnGPU(){
    if (requiredNumOfPoints==currNumOfPoints && (!EcgArea::instance().isRedrawNeeded())){
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER , valueBuffer);
    glBufferData(GL_ARRAY_BUFFER , sizeof(GLfloat)*requiredNumOfPoints, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER , 0);

    currNumOfPoints = requiredNumOfPoints;
    currWritePos=0;
    clear();
}

void Curve::moveNewDataToGPU(){
    int remaining=newPointBuffer.used();

    if (!remaining)
        return;

    glBindBuffer(GL_ARRAY_BUFFER , valueBuffer);
    while (remaining){
        GLfloat *buffer;

        int transferSize=std::min(newPointBuffer.getContinuousReadBuffer(buffer), remaining);
        transferSize=std::min(transferSize, (int)(currNumOfPoints-currWritePos));

        glBufferSubData(GL_ARRAY_BUFFER, currWritePos*sizeof(GLfloat), transferSize*sizeof(GLfloat), buffer);

        remaining -= transferSize;
        currWritePos += transferSize;

        if (currWritePos >= currNumOfPoints){
            currWritePos = 0;
        }

        newPointBuffer.skip(transferSize);
    }
}

void Curve::draw(){
    resizeOnGPU();
    moveNewDataToGPU();

    GLuint shaderProgram = ShaderBuilder::instance().useProgram(shaderId);

    glBindBuffer(GL_ARRAY_BUFFER, getXCoordinates(currNumOfPoints));
    glVertexAttribPointer(shader_a_Position, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(shader_a_Position);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniform2f(shader_screenSize, screenSize[0],screenSize[1]);
    glUniform3f(shader_position, position[0], position[1], zCoordinate);
    glUniform2f(shader_scale, scale[0], scale[1]);
    glUniform3f(shader_color, color[0], color[1], color[2]);

    glUniform1f(shader_endOffset, currWritePos);
    glUniform1f(shader_pointCount, currNumOfPoints);
    glUniform1f(shader_clearWidth, clearWidthInPoints);

    glBindBuffer(GL_ARRAY_BUFFER, valueBuffer);
    glVertexAttribPointer(shader_a_Value, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(shader_a_Value);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glLineWidth(3);
    glDrawArrays(GL_LINE_STRIP, 0, currNumOfPoints);
    glLineWidth(1);
}

void Curve::contextResized(int w, int h){
    screenSize.w = w;
    screenSize.h = h;
}

void Curve::setPosition(int x, int y) {
    position.x=x;
    position.y=y;
}

void Curve::setScale(float x, float y){
    scale.x=x;
    scale.y=y;

    setLength(lengthInPixels);
}

const Vec2 <int> Curve::endpointCoordinates(){
    return Vec2<int>(position.x + scale[0]*endCoordinates.x, position.y - scale[1]*endCoordinates.y );
}

void Curve::resetCurrWritePos() {
    currNumOfPoints = requiredNumOfPoints;
    currWritePos = requiredNumOfPoints;
    //endCoordinates.x = requiredNumOfPoints;
    //clear();
    reset = 1;
    // TODO is this really needed?
}
